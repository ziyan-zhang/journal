// SPDX-License-Identifier: GPL-2.0
// Per-core journaling part by Jongseok Kim
// SPDX-FileCopyrightText: Copyright (c) 2021 Sungkyunkwan University
/*
 *  linux/fs/ext4mj/fsync.c
 *
 *  Copyright (C) 1993  Stephen Tweedie (sct@redhat.com)
 *  from
 *  Copyright (C) 1992  Remy Card (card@masi.ibp.fr)
 *                      Laboratoire MASI - Institut Blaise Pascal
 *                      Universite Pierre et Marie Curie (Paris VI)
 *  from
 *  linux/fs/minix/truncate.c   Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  ext4mjfs fsync primitive
 *
 *  Big-endian to little-endian byte-swapping/bitmaps by
 *        David S. Miller (davem@caip.rutgers.edu), 1995
 *
 *  Removed unnecessary code duplication for little endian machines
 *  and excessive __inline__s.
 *        Andi Kleen, 1997
 *
 * Major simplications and cleanup - we only need to do the metadata, because
 * we can depend on generic_block_fdatasync() to sync the data blocks.
 */

#include <linux/time.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/blkdev.h>

#include "ext4mj.h"
#include "ext4mj_zj.h"

#include <trace/events/ext4mj.h>

/*
 * If we're not journaling and this is a just-created file, we have to
 * sync our parent directory (if it was freshly created) since
 * otherwise it will only be written by writeback, leaving a huge
 * window during which a crash may lose the file.  This may apply for
 * the parent directory's parent as well, and so on recursively, if
 * they are also freshly created.
 */
static int ext4mj_sync_parent(struct inode *inode)
{
	struct dentry *dentry = NULL;
	struct inode *next;
	int ret = 0;

	if (!ext4mj_test_inode_state(inode, EXT4MJ_STATE_NEWENTRY))
		return 0;
	inode = igrab(inode);
	while (ext4mj_test_inode_state(inode, EXT4MJ_STATE_NEWENTRY)) {
		ext4mj_clear_inode_state(inode, EXT4MJ_STATE_NEWENTRY);
		dentry = d_find_any_alias(inode);
		if (!dentry)
			break;
		next = igrab(d_inode(dentry->d_parent));
		dput(dentry);
		if (!next)
			break;
		iput(inode);
		inode = next;
		/*
		 * The directory inode may have gone through rmdir by now. But
		 * the inode itself and its blocks are still allocated (we hold
		 * a reference to the inode so it didn't go through
		 * ext4mj_evict_inode()) and so we are safe to flush metadata
		 * blocks and the inode.
		 */
		ret = sync_mapping_buffers(inode->i_mapping);
		if (ret)
			break;
		ret = sync_inode_metadata(inode, 1);
		if (ret)
			break;
	}
	iput(inode);
	return ret;
}

/*
 * akpm: A new design for ext4mj_sync_file().
 *
 * This is only called from sys_fsync(), sys_fdatasync() and sys_msync().
 * There cannot be a transaction open by this task.
 * Another task could have dirtied this inode.  Its data can be in any
 * state in the journalling system.
 *
 * What we do is just kick off a commit and wait on it.  This will snapshot the
 * inode to disk.
 */

int ext4mj_sync_file(struct file *file, loff_t start, loff_t end, int datasync)
{
	struct inode *inode = file->f_mapping->host;
	struct ext4mj_inode_info *ei = EXT4MJ_I(inode);
	zjournal_t *journal = EXT4MJ_SB(inode->i_sb)->s_journal;
	int ret = 0, err;
	tid_t commit_tid;
	bool needs_barrier = false;

	if (unlikely(ext4mj_forced_shutdown(EXT4MJ_SB(inode->i_sb))))
		return -EIO;

	J_ASSERT(ext4mj_journal_current_handle() == NULL);

	trace_ext4mj_sync_file_enter(file, datasync);

	if (sb_rdonly(inode->i_sb)) {
		/* Make sure that we read updated s_mount_flags value */
		smp_rmb();
		if (EXT4MJ_SB(inode->i_sb)->s_mount_flags & EXT4MJ_MF_FS_ABORTED)
			ret = -EROFS;
		goto out;
	}

	if (!journal) {
		ret = __generic_file_fsync(file, start, end, datasync);
		if (!ret)
			ret = ext4mj_sync_parent(inode);
		if (test_opt(inode->i_sb, BARRIER))
			goto issue_flush;
		goto out;
	}

	ret = file_write_and_wait_range(file, start, end);
	if (ret)
		return ret;
	/*
	 * data=writeback,ordered:
	 *  The caller's filemap_fdatawrite()/wait will sync the data.
	 *  Metadata is in the journal, we wait for proper transaction to
	 *  commit here.
	 *
	 * data=journal:
	 *  filemap_fdatawrite won't do anything (the buffers are clean).
	 *  ext4mj_force_commit will write the file data into the journal and
	 *  will wait on that.
	 *  filemap_fdatawait() will encounter a ton of newly-dirtied pages
	 *  (they were dirtied by commit).  But that's OK - the blocks are
	 *  safe in-journal, which is all fsync() needs to ensure.
	 */
	if (ext4mj_should_journal_data(inode)) {
		/*printk(KERN_ERR "fsync %d: %d\n", inode->i_ino, ei->i_sync_num);*/
		ret = ext4mj_force_commit_all(inode->i_sb);
		goto out;
	}

	commit_tid = datasync ? ei->i_datasync_tid : ei->i_sync_tid;
	if (journal->j_flags & ZJ_BARRIER &&
	    !zj_trans_will_send_data_barrier(journal, commit_tid))
		needs_barrier = true;
	ret = zj_complete_transaction(journal, commit_tid);
	if (needs_barrier) {
	issue_flush:
		err = blkdev_issue_flush(inode->i_sb->s_bdev, GFP_KERNEL, NULL);
		if (!ret)
			ret = err;
	}
out:
	trace_ext4mj_sync_file_exit(inode, ret);
	return ret;
}

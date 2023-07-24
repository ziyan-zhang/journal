/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BACKING_DEV_DEFS_H
#define __LINUX_BACKING_DEV_DEFS_H

#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/percpu_counter.h>
#include <linux/percpu-refcount.h>
#include <linux/flex_proportions.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kref.h>

struct page;
struct device;
struct dentry;

/*
 * Bits in bdi_writeback.state
 */
enum wb_state {
	WB_registered,		/* bdi_register() was done */
	WB_writeback_running,	/* Writeback is in progress */
	WB_has_dirty_io,	/* Dirty inodes on ->b_{dirty|io|more_io} */
};

enum wb_congested_state {
	WB_async_congested,	/* The async (write) queue is getting full */
	WB_sync_congested,	/* The sync queue is getting full */
};

typedef int (congested_fn)(void *, int);

enum wb_stat_item {
	WB_RECLAIMABLE,
	WB_WRITEBACK,
	WB_DIRTIED,
	WB_WRITTEN,
	NR_WB_STAT_ITEMS
};

#define WB_STAT_BATCH (8*(1+ilog2(nr_cpu_ids)))

/*
 * For cgroup writeback, multiple wb's may map to the same blkcg.  Those
 * wb's can operate mostly independently but should share the congested
 * state.  To facilitate such sharing, the congested state is tracked using
 * the following struct which is created on demand, indexed by blkcg ID on
 * its bdi, and refcounted.
 */
struct bdi_writeback_congested {
	unsigned long state;		/* WB_[a]sync_congested flags */
	atomic_t refcnt;		/* nr of attached wb's and blkg */

#ifdef CONFIG_CGROUP_WRITEBACK
	struct backing_dev_info *__bdi;	/* the associated bdi, set to NULL
					 * on bdi unregistration. For memcg-wb
					 * internal use only! */
	int blkcg_id;			/* ID of the associated blkcg */
	struct rb_node rb_node;		/* on bdi->cgwb_congestion_tree */
#endif
};

/*
 * Each wb (bdi_writeback) can perform writeback operations, is measured
 * and throttled, independently.  Without cgroup writeback, each bdi
 * (bdi_writeback) is served by its embedded bdi->wb.
 *
 * On the default hierarchy, blkcg implicitly enables memcg.  This allows
 * using memcg's page ownership for attributing writeback IOs, and every
 * memcg - blkcg combination can be served by its own wb by assigning a
 * dedicated wb to each memcg, which enables isolation across different
 * cgroups and propagation of IO back pressure down from the IO layer upto
 * the tasks which are generating the dirty pages to be written back.
 *
 * A cgroup wb is indexed on its bdi by the ID of the associated memcg,
 * refcounted with the number of inodes attached to it, and pins the memcg
 * and the corresponding blkcg.  As the corresponding blkcg for a memcg may
 * change as blkcg is disabled and enabled higher up in the hierarchy, a wb
 * is tested for blkcg after lookup and removed from index on mismatch so
 * that a new wb for the combination can be created.
 * 
 * 每个wb（bdi_writeback）可以独立地执行回写操作，可以独立地被测量和限制。
 * 在没有cgroup writeback的情况下，每个bdi（bdi_writeback）都由其嵌入的bdi->wb提供服务。
 * 
 * 在默认层次结构上，blkcg隐式启用memcg。这允许使用memcg的页面所有权来归属回写IO，
 * 并且每个memcg - blkcg组合都可以通过为每个memcg分配专用的wb以使用其自己的wb提供服务，
 * 这使得可以跨不同的cgroup进行隔离，并将IO回压从IO层向下传播到生成脏页面的任务。
 * 
 * cgroup wb在其bdi上由关联的memcg的ID索引，与附加到它的inode的数量一起引用，并固定memcg和相应的blkcg。
 * 由于memcg的相应blkcg可能会更改，因为blkcg在层次结构中更高的位置被禁用和启用，因此在查找后会测试blkcg的wb，
 * 并在不匹配时从索引中删除，以便可以为组合创建新的wb。
 */
struct bdi_writeback {
	struct backing_dev_info *bdi;	/* our parent bdi */

	unsigned long state;		/* Always use atomic bitops on this */
	unsigned long last_old_flush;	/* last old data flush */

	struct list_head b_dirty;	/* dirty inodes */
	struct list_head b_io;		/* parked for writeback */
	struct list_head b_more_io;	/* parked for more writeback */
	struct list_head b_dirty_time;	/* time stamps are dirty */
	spinlock_t list_lock;		/* protects the b_* lists */

	struct percpu_counter stat[NR_WB_STAT_ITEMS];

	struct bdi_writeback_congested *congested;

	unsigned long bw_time_stamp;	/* last time write bw is updated */
	unsigned long dirtied_stamp;
	unsigned long written_stamp;	/* pages written at bw_time_stamp */
	unsigned long write_bandwidth;	/* the estimated write bandwidth */
	unsigned long avg_write_bandwidth; /* further smoothed write bw, > 0 */

	/*
	 * The base dirty throttle rate, re-calculated on every 200ms.
	 * All the bdi tasks' dirty rate will be curbed under it.
	 * @dirty_ratelimit tracks the estimated @balanced_dirty_ratelimit
	 * in small steps and is much more smooth/stable than the latter.
	 * 
	 * 基本的脏限制速率，每200ms重新计算一次。
	 * 所有bdi任务的脏速率都将在其下。
	 * @dirty_ratelimit跟踪小步骤中估计的@balanced_dirty_ratelimit，
	 * 并且比后者更平滑/稳定。
	 */
	unsigned long dirty_ratelimit;
	unsigned long balanced_dirty_ratelimit;

	struct fprop_local_percpu completions;
	int dirty_exceeded;

	spinlock_t work_lock;		/* protects work_list & dwork scheduling */
	struct list_head work_list;
	struct delayed_work dwork;	/* work item used for writeback */

	unsigned long dirty_sleep;	/* last wait */

	struct list_head bdi_node;	/* anchored at bdi->wb_list */

#ifdef CONFIG_CGROUP_WRITEBACK
	struct percpu_ref refcnt;	/* used only for !root wb's */
	struct fprop_local_percpu memcg_completions;
	struct cgroup_subsys_state *memcg_css; /* the associated memcg */
	struct cgroup_subsys_state *blkcg_css; /* and blkcg */
	struct list_head memcg_node;	/* anchored at memcg->cgwb_list */
	struct list_head blkcg_node;	/* anchored at blkcg->cgwb_list */

	union {
		struct work_struct release_work;
		struct rcu_head rcu;
	};
#endif
};

struct backing_dev_info {
	struct list_head bdi_list;
	unsigned long ra_pages;	/* max readahead in PAGE_SIZE units */
	unsigned long io_pages;	/* max allowed IO size */
	congested_fn *congested_fn; /* Function pointer if device is md/dm */
	void *congested_data;	/* Pointer to aux data for congested func */

	const char *name;

	struct kref refcnt;	/* Reference counter for the structure */
	unsigned int capabilities; /* Device capabilities */
	unsigned int min_ratio;
	unsigned int max_ratio, max_prop_frac;

	/*
	 * Sum of avg_write_bw of wbs with dirty inodes.  > 0 if there are
	 * any dirty wbs, which is depended upon by bdi_has_dirty().
	 */
	atomic_long_t tot_write_bandwidth;

	struct bdi_writeback wb;  /* the root writeback info for this bdi */
	struct list_head wb_list; /* list of all wbs */
#ifdef CONFIG_CGROUP_WRITEBACK
	struct radix_tree_root cgwb_tree; /* radix tree of active cgroup wbs */
	struct rb_root cgwb_congested_tree; /* their congested states */
	struct mutex cgwb_release_mutex;  /* protect shutdown of wb structs */
#else
	struct bdi_writeback_congested *wb_congested;
#endif
	wait_queue_head_t wb_waitq;

	struct device *dev;
	struct device *owner;

	struct timer_list laptop_mode_wb_timer;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debug_dir;
	struct dentry *debug_stats;
#endif
};

enum {
	BLK_RW_ASYNC	= 0,
	BLK_RW_SYNC	= 1,
};

void clear_wb_congested(struct bdi_writeback_congested *congested, int sync);
void set_wb_congested(struct bdi_writeback_congested *congested, int sync);

static inline void clear_bdi_congested(struct backing_dev_info *bdi, int sync)
{
	clear_wb_congested(bdi->wb.congested, sync);
}

static inline void set_bdi_congested(struct backing_dev_info *bdi, int sync)
{
	set_wb_congested(bdi->wb.congested, sync);
}

struct wb_lock_cookie {
	bool locked;
	unsigned long flags;
};

#ifdef CONFIG_CGROUP_WRITEBACK

/**
 * wb_tryget - try to increment a wb's refcount
 * @wb: bdi_writeback to get
 */
static inline bool wb_tryget(struct bdi_writeback *wb)
{
	if (wb != &wb->bdi->wb)
		return percpu_ref_tryget(&wb->refcnt);
	return true;
}

/**
 * wb_get - increment a wb's refcount
 * @wb: bdi_writeback to get
 */
static inline void wb_get(struct bdi_writeback *wb)
{
	if (wb != &wb->bdi->wb)
		percpu_ref_get(&wb->refcnt);
}

/**
 * wb_put - decrement a wb's refcount
 * @wb: bdi_writeback to put
 */
static inline void wb_put(struct bdi_writeback *wb)
{
	if (wb != &wb->bdi->wb)
		percpu_ref_put(&wb->refcnt);
}

/**
 * wb_dying - is a wb dying?
 * @wb: bdi_writeback of interest
 *
 * Returns whether @wb is unlinked and being drained.
 */
static inline bool wb_dying(struct bdi_writeback *wb)
{
	return percpu_ref_is_dying(&wb->refcnt);
}

#else	/* CONFIG_CGROUP_WRITEBACK */

static inline bool wb_tryget(struct bdi_writeback *wb)
{
	return true;
}

static inline void wb_get(struct bdi_writeback *wb)
{
}

static inline void wb_put(struct bdi_writeback *wb)
{
}

static inline bool wb_dying(struct bdi_writeback *wb)
{
	return false;
}

#endif	/* CONFIG_CGROUP_WRITEBACK */

#endif	/* __LINUX_BACKING_DEV_DEFS_H */

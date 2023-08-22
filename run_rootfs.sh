qemu-system-x86_64 -s -S \
	-smp 2 -m 2048 -nographic -serial mon:stdio \
	-kernel Linux_compiled/arch/x86_64/boot/bzImage \
	--append "console=ttyS0 root=/dev/sda init=/linuxrc rw" \
	-hda /home/zy/lab/rootfs.ext4 \
	-hdb /home/zy/lab/swap_10g.img \
	-drive id=nvme0,if=none,format=qcow2,file=/home/zy/lab/ext2.qcow2 \
	-device nvme,drive=nvme0,serial=001,max_ioqpairs=16 \
	-drive id=nvme1,if=none,format=qcow2,file=/home/zy/lab/ext4.qcow2 \
	-device nvme,drive=nvme1,serial=002,max_ioqpairs=16 \
	-drive id=nvme2,if=none,format=qcow2,file=/home/zy/lab/ext4_mj.qcow2 \
	-device nvme,drive=nvme2,serial=002,max_ioqpairs=16 \


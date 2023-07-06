qemu-system-x86_64 -s -S\
	-nographic -serial mon:stdio -smp 2 -m 2048 \
	-kernel Linux_compiled/arch/x86_64/boot/bzImage \
	-hda ~/lab/ubuntu.img \
	-append "root=/dev/sda2 rw console=ttyS0" \
	-hdb ~/lab/swap_ext4.img \
	-hdc ~/lab/exe_mj.img

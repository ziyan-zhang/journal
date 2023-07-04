qemu-system-x86_64 \
	-kernel Linux_compiled/arch/x86_64/boot/bzImage \
	-append "root=/dev/sda2 rw console=ttyS0" \
	-hda ~/lab/ubuntu.img \
	-enable-kvm \
	-nographic \
	-smp 2 -m 2048 \
 	-serial mon:stdio \

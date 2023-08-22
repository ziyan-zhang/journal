file Linux_compiled/vmlinux
target remote :1234
c
add-symbol-file Linux_compiled/fs/ext4mj/ext4mj.ko 0xffffffffa0028000
add-symbol-file Linux_compiled/fs/zj/zj.ko 0xffffffffa0000000
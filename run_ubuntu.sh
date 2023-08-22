###
 # @Author: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @Date: 2023-07-29 16:01:39
 # @LastEditors: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @LastEditTime: 2023-07-29 18:23:06
 # @FilePath: /z-journal/run_ubuntu.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
qemu-system-x86_64 -m 8192 -smp 6 --enable-kvm ./ubuntu_20g.img \
	-cpu host -serial mon:stdio \
	-hdb ./ext4.swap \
	-hdc ./sdc.img \
	-drive id=nvme0,if=none,format=qcow2,file=./exe.img \
	-device nvme,drive=nvme0,serial=000,max_ioqpairs=16 \
	-drive id=nvme1,if=none,format=qcow2,file=./ext4.qcow2 \
	-device nvme,drive=nvme1,serial=001,max_ioqpairs=16 \
	-drive id=nvme2,if=none,format=qcow2,file=./ext2.qcow2 \
	-device nvme,drive=nvme2,serial=002,max_ioqpairs=16 \
	-net user,hostfwd=::2222-:22 -net nic

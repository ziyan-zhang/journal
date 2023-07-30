###
 # @Author: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @Date: 2023-07-29 16:01:39
 # @LastEditors: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @LastEditTime: 2023-07-29 18:23:06
 # @FilePath: /z-journal/run_ubuntu.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
qemu-system-x86_64 -m 2048 --enable-kvm ~/lab/ubuntu.img \
	-hdb ~/lab/swap_ext4.img \
	-device nvme,drive=nvme1,serial=deadbeaf,max_ioqpairs=8 \
	-drive file=/home/zy/lab/ext2.qcow2,format=qcow2,if=none,cache=writeback,id=nvme1 \
	-device nvme,drive=nvme2,serial=deadbeaf,max_ioqpairs=8 \
	-drive file=/home/zy/lab/ext4.qcow2,format=qcow2,if=none,cache=writeback,id=nvme2 \
	-device nvme,drive=nvme3,serial=deadbeaf,max_ioqpairs=8 \
	-drive file=/home/zy/lab/ext4_mj.qcow2,format=qcow2,if=none,cache=writeback,id=nvme3 \

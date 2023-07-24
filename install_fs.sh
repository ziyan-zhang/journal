#!/bin/bash

###
 # @Author: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @Date: 2023-07-11 09:11:37
 # @LastEditors: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @LastEditTime: 2023-07-16 20:03:10
 # @FilePath: /z-journal/install_fs.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 

# 挂载交换盘和非模块文件系统盘到目录
sudo mount /dev/sdb ~/swap_dir/
sudo mount /dev/sdd ~/scsi_ext2/
sudo mount /dev/nvme0n1 ~/nvme_ext2/
sudo mount /dev/nvme1n1 ~/nvme_ext4/

# 安装z-journal模块
sudo insmod ~/swap_dir/zj.ko
sudo insmod ~/swap_dir/ext4mj.ko
# 格式化z-journal盘
sudo ./e2fsprog-zj/misc/mke2fs -t ext4 -J multi_journal -F -G 1 /dev/sdc
sudo ./e2fsprog-zj/misc/tune2fs -o journal_data /dev/sdc
# 挂载z-journal盘
sudo mount -t ext4mj /dev/sdc ~/zj_dir/

df -T
sudo cp /home/zy/swap_dir/*.cpp ~/zj_dir/
sudo cp /home/zy/swap_dir/*.cpp ~/scsi_ext2/
sudo cp /home/zy/swap_dir/*.cpp ~/nvme_ext2/
sudo cp /home/zy/swap_dir/*.cpp ~/nvme_ext4/

echo "注意执行./write_to_mytest要用管理员权限"

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
sudo mount /dev/sdb /home/zy/swap_dir/
sudo mount /dev/nvme0n1p1 /home/zy/ext4mj_dir/
sudo mount /dev/nvme1n1 /home/zy/ext4_dir/
sudo mount /dev/nvme2n1 /home/zy/ext2_dir/

# 安装z-journal模块
sudo insmod /home/zy/swap_dir/zj.ko
sudo insmod /home/zy/swap_dir/ext4mj.ko
# 格式化z-journal盘
sudo /home/zy/e2fsprog-zj/misc/mke2fs -t ext4 -J multi_journal -F -G 1 /dev/nvme0n1p1
sudo /home/zy/e2fsprog-zj/misc/tune2fs -o journal_data /dev/nvme0n1p1
# 挂载z-journal盘
sudo mount -t ext4mj /dev/sdc /home/zy/ext4mj_dir/

df -T
sudo cp /home/zy/swap_dir/*.cpp /home/zy/ext4mj_dir/
sudo cp /home/zy/swap_dir/*.cpp /home/zy/ext4_dir/
sudo cp /home/zy/swap_dir/*.cpp /home/zy/ext2_dir/

echo "注意执行./write_to_mytest要用管理员权限"

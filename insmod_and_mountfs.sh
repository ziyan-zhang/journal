#!/bin/bash

###
 # @Author: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @Date: 2023-07-11 09:11:37
 # @LastEditors: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 # @LastEditTime: 2023-07-11 09:58:18
 # @FilePath: /z-journal/install_fs.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 


sudo insmod /home/zy/sdb_dir/zj.ko
sudo insmod /home/zy/sdb_dir/ext4mj.ko
sudo mount /dev/sdc /home/zy/exe_dir
df -T

cd /home/zy/ext2_dir/
sudo cp /home/zy/sdb_dir/write_to_mytest.cpp ./
sudo g++ write_to_mytest.cpp -o write_to_mytest -g

cd /home/zy/exe_dir/
sudo cp /home/zy/sdb_dir/write_to_mytest.cpp ./
sudo g++ write_to_mytest.cpp -o write_to_mytest -g

echo "执行./write_to_mytest要用管理员权限"
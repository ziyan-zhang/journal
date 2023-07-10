/*
 * @Author: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 * @Date: 2023-07-07 15:45:04
 * @LastEditors: Ziyan ZHANG zhangzy273@mail2.sysu.edu.cn
 * @LastEditTime: 2023-07-10 16:20:21
 * @FilePath: /play_ground/write_to_mytest.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */


#include <iostream>
#include <fcntl.h>
#include <unistd.h>

int main() {
    std::string filename = "example.txt";
    std::string content = "This is the content of the file.";

    // 打开文件，使用O_CREAT标志创建新文件
    int fileDescriptor = open(filename.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if (fileDescriptor != -1) {
        // 写入字符串到文件
        ssize_t bytesWritten = write(fileDescriptor, content.c_str(), content.length());

        // if (bytesWritten != -1) {
        //     std::cout << "文件已保存." << std::endl;
        // } else {
        //     std::cout << "写入文件时出现错误." << std::endl;
        // }

        // 关闭文件描述符
        close(fileDescriptor);
    } else {
        std::cout << "无法创建文件." << std::endl;
    }

    return 0;
}

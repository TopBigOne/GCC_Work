#include <iostream>
#include <fstream>
#include <iterator>

#include <fcntl.h>
#include <unistd.h>

using namespace std;

void test_1();


void test_1() {
    int fileDescriptor = open("/Users/dev/Documents/GCC_Work/CPP/dup2/output.txt", O_WRONLY | O_CREAT,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fileDescriptor < 0) {
        perror(" open in ERROR.");
        return;
    }

    // 备份原来的标准输出描述符
    int originalStdout = dup(STDOUT_FILENO);


    int result = dup2(fileDescriptor, STDOUT_FILENO);
    if (result < 0) {
        perror(" dup2 in ERROR.");
        return;
    }
    close(fileDescriptor);

    std::cout << " 陈筱雅的身材蛮好的..." << std::flush;
    // 给标准输出fd 重新赋值
    dup2(originalStdout, STDOUT_FILENO);
    close(originalStdout);
    // 在恢复后再次输出到标准输出
    std::cout << " This will be displayed on the console." << std::endl;
}


int main() {

    test_1();
    return 0;
}


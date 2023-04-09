#include <iostream>


struct int_char {
    int  x; // 4 个字节
    char y; // 1 个字节
} INT_CHAR;

int main() {
    // 结果却是 8个字节；
    printf("finally size : %lu", sizeof(INT_CHAR));
    return 0;
}

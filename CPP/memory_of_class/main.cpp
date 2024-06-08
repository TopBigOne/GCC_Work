#include <iostream>

#include "A.h"
#include "B.h"
#include "B_1.h"
#include "C.h"

int main() {
    std::cout << "sizeof（int) = " << sizeof(int) << std::endl;
    A a;
    // 一共是12byte，但是对齐是8，所以按照8的倍数，16 来取整
    std::cout << "sizeof（A) = " << sizeof(a) << std::endl;
    B b;
    // 正好是16byte
    std::cout << "sizeof（B) = " << sizeof(b) << std::endl;
    B_1 b_1;
    // 正好是16byte
    std::cout << "sizeof（B_1) = " << sizeof(B_1) << std::endl;


    C c;
    // 正好是16byte
    std::cout << "sizeof（C) = " << sizeof(c) << std::endl;




    return 0;
}

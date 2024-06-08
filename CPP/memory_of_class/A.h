//
// Created by dev on 2024/6/8.
//

#ifndef MEMORY_OF_CLASS_A_H
#define MEMORY_OF_CLASS_A_H


/**
 * 12 byte
 */
class A {
public:
    //alignas(8)
    // 4 byte
    int a;
    // 虚函数表指针的大小（通常为8个字节，因为你提到内存对齐是8），总共为12个字节。
    // 然而，由于内存对齐规则，结构体或类的大小通常会向上取整到最接近的对齐单位的倍数。在这种情况下，对齐单位是8个字节，所以sizeof(A)会被调整为16个字节。
    // 8 byte
    virtual void v();

};


#endif //MEMORY_OF_CLASS_A_H

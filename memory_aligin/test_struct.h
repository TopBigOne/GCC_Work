//
// Created by DEV on 2025/11/26.
//

#ifndef MEMORY_ALIGIN_TEST_STRUCT_H
#define MEMORY_ALIGIN_TEST_STRUCT_H

struct int_char {
    int  x; // 4 个字节
    char y; // 1 个字节
} INT_CHAR;


#ifdef __cplusplus
// test_struct.c 是 C 文件，编译后符号名为 _test_two（无修饰）
// main.cpp 是 C++ 文件，调用 test_two() 时链接器会找 _test_two__Z8test_twov 这样的修饰名
// extern "C" 告诉 C++ 编译器按 C 方式链接，不做名称修饰
extern "C" {
#endif

void test_one();
void test_two();

#ifdef __cplusplus
}
#endif


#endif //MEMORY_ALIGIN_TEST_STRUCT_H
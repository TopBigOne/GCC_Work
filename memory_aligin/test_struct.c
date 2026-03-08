//
// Created by DEV on 2025/11/26.
//
#include "test_struct.h"
#include <stdlib.h>
#include <stdio.h>

void test_one() {
    // 结果却是 8个字节；
    printf("finally size : %lu\n", sizeof(INT_CHAR));
}

void test_two() {
    int *p = malloc(sizeof(int));   // 申请 4 字节堆内存
    *p = 100;                       // 存入整数 100
    uint8_t *p2 = p;                // 用 uint8_t* 指向同一块内存
    printf("p2 : %p\n", p2);       // 打印 p2（实际打印的是指针地址，有 bug）

}
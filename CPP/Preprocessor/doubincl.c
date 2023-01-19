//
// Created by dev on 2022/12/29.
//
#include "names.h"
#include "names.h"
#include "doubincl.h"
#include <stdio.h>
#include <strings.h>

void start_test_double_include() {
    puts("测试：double 次引用头文件");
    names winner = {"less", "Ismoor"};
    char info[30] = "The winner is :";
    strcat(info, winner.first);
    strcat(info, " ");
    strcat(info, winner.last);
    printf("info : %s", info);
}

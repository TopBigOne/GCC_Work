//
// Created by dev on 2022/12/29.
//

#include "ifdef.h"
#include <stdio.h>

#define JUST_CHECKING
#define LIMIT 4

int start_ifdef() {
    puts("使用条件编译");
    int i;
    int total = 0;
    for (i = 0; i < LIMIT; ++i) {
        total += 2 * i * i + 1;
#ifdef JUST_CHECKING
        printf("| ------------------------------↓\n");
        printf("| i = %d ,running total = %d\n", i, total);
        printf("| ------------------------------↑\n");
#endif
        printf("Grand total = %d\n", total);
    }

}
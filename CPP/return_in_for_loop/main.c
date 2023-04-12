#include <stdio.h>

void test_double_for();

int main() {
    test_double_for();
    return 0;
}

void test_double_for() {
    puts("test_double_for : start.");
    for (int i = 0; i < 4; i++) {
        printf("i==> %d \n", i);

        for (int j = 0; j < 3; j++) {
            if (i == 2) {
                puts(" 在第二层for中，直接结束整个函数.");
                return;
            }
            printf(" j==> %d \n", j);
        }
        printf("\n");
    }

    // NOTE: 下面这一行代码，是不会走的.
    puts("test_double_for : end.");
}

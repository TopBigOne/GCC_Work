#include <stdio.h>

#include "main_fun.h"

void testOne();

int add(int a, int b) {
    return a + b;
}

int main() {
    testOne();
    return 0;
}

void testOne() {
    int sumResult = add(1, 2);
    printf("sumResult: %d", sumResult);

}

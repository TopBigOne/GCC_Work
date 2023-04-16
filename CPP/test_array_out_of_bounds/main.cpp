#include <iostream>

#define MY_SIZE 11

int array[MY_SIZE] = {25, 65, 68, 454,
                      67, 454, 78, 454,
                      76, 45, 35};

void test_one();

int main() {
    test_one();
    return 0;
}

void test_one() {
    int index      = 2;
    // 我也装逼，%一下；
    int real_index = index % MY_SIZE;
    printf("the really index is : %d\n", real_index);
    int result = array[real_index];
    printf("the index %d value is : %d\n", index, result);
    puts("-----ok ,,,,下面，我来测试一下数组越界-------");
    index      = 110;
    real_index = index % MY_SIZE;
    result     = array[real_index];
    printf("the really bound index is : %d\n", real_index);
    printf("the  bound index array value  is : %d\n", result);


}

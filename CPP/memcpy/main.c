#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char src[100] = "钱塘自古繁华，云树绕堤沙，有三秋桂子，十里荷花";

void test_one();

void test_two();

void test_three();

void test_four();

int main() {
    test_one();
    test_two();
    test_three();
    test_four();
    return 0;
}

void test_four() {
    puts("case 4: memcpy 覆盖原有数据.");
    static char *src  = "98765天气不错哦";
    char        *dest = "土豆丝";
    memcpy(dest, src, strlen(src));

    printf("dest is after memcpy a : %s\n", dest);

}

void test_three() {
    puts("case 3:\n 对指针++做测试：");
    static char *src     = "123456你可别这样了";
    char        *des_one = malloc(0);

    memcpy(des_one, src, strlen(src) + 1);
    printf("des_one is : %s\n", des_one);
    memset(des_one, 0, strlen(des_one));
    printf("des_one is after memset : %s\n", des_one);
    memcpy(des_one, src + 3, strlen(src) + 10);
    printf("des_one is after memcpy again : %s\n", des_one);
    free(des_one);
}


void test_one() {
    puts("cas1 : \n将字符串复制到数组dest中:");
    char dest[100];
    memcpy(dest, src, strlen(src) + 1);
    printf("dest is : %s\n", dest);
}


void test_two() {
    puts("case 2: \n从指针的第 6 个字符，开始复制，连续复制3个字符：");
    char des[100];
    memcpy(des, src + 6, 6);
    des[6] = '\0';
    printf("从指针的第六个字符 赋值的结果 : %s \n", des);
}

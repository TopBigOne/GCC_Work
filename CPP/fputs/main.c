#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

const char *filePath = "/Users/dev/Documents/GCC_Work/CPP/fputs/test.text";

void testOne();

void testTwo();

void testThree();

int main() {

    testOne();
    testTwo();
    testThree();

    return 0;
}

void testThree() {
    puts("测试：strncat() \n");
    puts("参考网址:https://www.runoob.com/cprogramming/c-function-strncat.html \n");

    FILE *fp;
    fp = fopen(filePath, "w");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }
    char temp[100] = {0};
    char *status = "我很累，但是我还需要坚持";
    strncat(temp, status, strlen(status));
    printf("now the temp content is : %s", temp);

}


void testOne() {
    FILE *fp;
    fp = fopen(filePath, "w");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }
    // 往文件中写入字符串
    fputs("stdin", fp);
    fclose(fp);
}

void testTwo() {
    FILE *fp;
    fp = fopen(filePath, "at+");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }
    fputs("hello ..,,吃了吗？", fp);
    fflush(fp);

    char str[102] = {0}, strTemp[100];


    printf("please input a string:\n");
    gets(strTemp);
    printf("your  input content is : %s\n", strTemp); // ok

    // 拼接一个换行符号
    strcat(str, "\n");
    strcat(str, strTemp);
    strcat(str, "你好牛逼");
    strcat(str, "\n");
    fputs(str, fp);

    fclose(fp);
}

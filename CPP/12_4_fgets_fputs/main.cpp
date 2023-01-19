#include <iostream>

auto filePath = "/Users/dev/Documents/GCC_Work/CPP/12_4_fgets_fputs/temp.text";

#define LENGTH 1000


void test_fgets();

void test_fputs();

int main() {
    test_fgets();
    test_fputs();
    return 0;
}


void test_fgets() {
    puts("测试：读字符串函数 fgets");
    FILE *fp = fopen(filePath, "r");
    if (fp == nullptr) {
        puts("the fp is null.");
        exit(0);
    }
    char str[LENGTH + 1];
    char *result = fgets(str, LENGTH, fp);
    printf("result : %s", result);
}

void test_fputs() {
    puts("测试：读字符串函数 fputs");

    char str[102] = {0};
    char strTemp[100];


    auto filePath2 = "/Users/dev/Documents/GCC_Work/CPP/12_4_fgets_fputs/temp2.text";

    FILE *fp = fopen(filePath2, "at+");
    if (fp == nullptr) {
        puts("the fp is null.");
        exit(0);
    }
    puts("please input a string：");
    // 获取 你在命令行中输入的内容：
    gets(strTemp);
    printf("your input content is : %s", strTemp);
    strcat(str, "\n");
    strcat(str, strTemp);
    fputs(str, fp);
    fclose(fp);


}

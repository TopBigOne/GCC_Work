#include <stdio.h>
#include <strings.h>

void testOne();

void testTwo();

int main() {
    testOne();
    testTwo();
    return 0;
}

void testTwo() {
    puts("我好奇，想测试一下换行符\n");
    const char str[] = "你牛逼,\n你过来啊🤣🤣";
    const char ch = '\n';
    char *result = strchr(str, ch);
    printf("换行符号后的内容:%s", result);


}

void testOne() {
    const char str[] = "http://www.runoob.com";
    const char ch = '.';
    char *result = strchr(str, ch);
    printf("result:%s", result);
}

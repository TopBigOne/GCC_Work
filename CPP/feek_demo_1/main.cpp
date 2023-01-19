#include <iostream>


#define CNTL_Z '\032'/*DOS 文件结尾标记*/
#define SLEN 81

void test_fseek();


auto filePath = "/Users/dev/Documents/GCC_Work/CPP/feek_demo_1/temp.text";

int main() {
    test_fseek();
    return 0;
}

void test_fseek() {
    puts("start invoke fseek");
    FILE *fp = fopen(filePath, "rb");
    long count;
    long last;
    char ch;
    if (fp == nullptr) {
        printf("reverse can`t open %s\n", filePath);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0L, SEEK_END);/*定位到文件末尾*/
    last = ftell(fp);
    printf("last :%ld\n", last);
    for (count = 1L; count <= last; count++) {
        fseek(fp, -count, SEEK_END);/*回退 count 个字节*/
        ch = getc(fp);
        if (ch != CNTL_Z && ch != '\r') {
            putchar(ch);
            putchar('\n');
        }

    }
    putchar('\n');
    fclose(fp);


}

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

void writeToFile();

void displayFile();

void testStudent();


void test_with_pointer_jia_jia();

const char *filePath = "/Users/dev/Documents/GCC_Work/CPP/fread/test.txt";
typedef struct Student {
    char num[8];
    char name[10];
}          st;

int main() {
    writeToFile();
    displayFile();

    testStudent();

    test_with_pointer_jia_jia();
    return 0;
}


void testStudent() {
    puts("step 3: \n testStudent() : 测试 fwrite ");
    st s[3] = {
            "1001", "wang",
            "1002", "li",
            "1003", "chen",
    };

    FILE *fp = fopen(filePath, "w");
    if (fp == NULL) {
        return;
    }

    for (int i = 0; i < 3; ++i) {
        fwrite(&s[i], sizeof(st), 1, fp);
    }
    fclose(fp);

    puts("开始测试 fread:");
    FILE *readfp = fopen(filePath, "r");
    if (readfp == NULL) {
        return;
    }


    st       readStudent[3];
    for (int i = 0; i < 3; ++i) {
        fread(&readStudent[i], sizeof(st), 1, readfp);
    }
    fclose(readfp);

    for (int i = 0; i < 3; ++i) {
        puts(readStudent[i].name);
        puts(readStudent[i].num);
        puts("---------------");
    }

}


void writeToFile() {
    puts("step 1: \nwriteToFile()");
    FILE *fp;
    fp = fopen(filePath, "w+");
    const char *tempContent = "hello,everyone!";


    fwrite(tempContent, strlen(tempContent) + 1, 1, fp);
    fclose(fp);
}


void displayFile() {
    puts("step 2: \ndisplayFile():");
    FILE *fp;
    fp = fopen(filePath, "rt");
    //求得文件的大小
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    printf("file size : %ld\n", size);
    rewind(fp);

    //申请一块能装下整个文件的空间
    char *ar = (char *) malloc(sizeof(char) * size);
    //读文件，每次读一个，共读size次
    fread(ar, 3, 1, fp);

    printf("ar content is : %s\n", ar);

    fclose(fp);
    free(ar);

}


void test_with_pointer_jia_jia() {
    puts("test_with_pointer_jia_jia : ");
    static char *file_path = "/Users/dev/Documents/GCC_Work/CPP/fread/test_2.txt";
    FILE        *fp        = fopen(file_path, "rt");
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    printf(" file_size is : %ld\n", file_size);

    char *buffer = malloc(sizeof(char) * 1000);
    memset(buffer, 0, sizeof(char));
    long read_result = fread(buffer, sizeof(char), 3, fp);
    printf(" read_result is    : %ld\n", read_result);
    printf(" buffer content is : %s\n", buffer);
    rewind(fp);
    // 不加的话 ，会直接重头开始
    read_result = fread(buffer+3, sizeof(char), 2, fp);
    printf(" read_result is    : %ld\n", read_result);
    printf(" buffer content is : %s\n", buffer);


}

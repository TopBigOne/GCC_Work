#include <stdio.h>

#include <stdlib.h>
#include <strings.h>

#define  BUFSIZE 4096
#define SLEN 81

char *file_app = "/Users/dev/Documents/GCC_Work/CPP/13_5_append/file_app.txt";

char *file_src = "/Users/dev/Documents/GCC_Work/CPP/13_5_append/file_src.txt";


void start_append();

void append(FILE *source, FILE *dest);

char *s_gets(char *st, int n);


int main() {
    start_append();
    return 0;
}

void start_append() {
    puts("start append file");
    FILE *fa, *fs;
    int files = 0;// 附件文件数量
    int ch;
    if ((fa = fopen(file_app, "a+")) == NULL) {
        fprintf(stderr, "can`t open %s\n", file_app);
        exit(EXIT_FAILURE);
    }
    // todo setvbuf 是干嘛的？
    if (setvbuf(fa, NULL, _IOFBF, BUFSIZE) != 0) {
        fputs("can`t create output buffer\n", stderr);
        exit(EXIT_FAILURE);
    }

    // todo s_gets 干嘛的？
    while (s_gets(file_src, SLEN) && file_src[0] != '\0') {
        // 路径比较
        if (strcmp(file_src, file_app) == 0) {
            fputs("can`t append file to itself\n", stderr);
        } else if ((fs = fopen(file_src, "r")) == NULL) {
            fprintf(stderr, "can`t open %s\n", file_src);
        } else {
            if (setvbuf(fs, NULL, _IOFBF, BUFSIZE) != 0) {
                fputs("can`t create input buffer\n", stderr);
                continue;
            }
            puts("start invoke append() function");
            append(fs, fa);
            if (ferror(fs) != 0) {
                fprintf(stderr, "Error in reading file %s\n", file_src);
            }
            // todo ferror 是干嘛的？
            if (ferror(fa) != 0) {
                fprintf(stderr, "Error in reading file %s\n", file_app);
            }

            fclose(fs);
            files++;
            printf("File %s append.\n", file_src);
            puts("Next file (empty line to quit):");
        }
    }

    printf("Done appending,%d files appended.\n", files);
    // todo rewind 是干嘛的？
    rewind(fa);
    printf("%s contents is: \n", file_app);

    // todo getc 是干嘛的
    // todo  putchar 是干嘛的
    while ((ch = getc(fa)) != EOF) {
        putchar(ch);
    }
    puts("\n");
    puts("Done displaying.");
    fclose(fa);
    puts(" append file over");
}

void append(FILE *source, FILE *dest) {
    puts("start invoke append");
    size_t bytes;
    static char temp[BUFSIZ];// 只分配一次
    while ((bytes = fread(temp, sizeof(char), BUFSIZ, source)) > 0) {
        fwrite(temp, sizeof(char), bytes, dest);
    }
}

char *s_gets(char *st, int n) {
    puts("start invoke s_gets");
    char *ret_val;
    char *find;
    // fgets: 获取字符串
    ret_val = fgets(st, n, stdin);

    if (ret_val) {
        // strchr:换行以后得字符
        find = strchr(st, '\n');// 查找换行符
        if (find) {// 如果地址不是NULL
            *find = '\0';// 在此处放置一个空字串
        } else {
            while (getchar() != '\n') {
                continue;
            }
        }

    }
    return ret_val;
}


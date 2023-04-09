#include <stdio.h>

#define FILE_PATH "/Users/dev/Documents/GCC_Work/CPP/fseek/file.txt"

int my_code();

int main() {
    int result = my_code();
    printf("result : %d", result);
    return 0;
}

int my_code() {
    int  result = -1;
    FILE *fp    = NULL;
    fp = fopen(FILE_PATH, "wb+");
    if (!fp) {
        fprintf(stderr, "can`nt open file.");
        fclose(fp);
        return result;
    }
    // case 1:移动到文件开始的位置+7；
    result = fseek(fp, 7, SEEK_SET);
    if (result != 0) {
        return result;
    }
    fputs("12346", fp);

    // case 2:移动到文件的末尾位置：
    result = fseek(fp, 0, SEEK_END);
    fputs("qazx", fp);

    // case 3:移动到文件的当前位置，再往回走3个位置：
    // 注意： 会把原来3个位置的字符替换掉；
    result = fseek(fp, -3, SEEK_CUR);
    fputs("888888", fp);


    fclose(fp);

    return result;


}

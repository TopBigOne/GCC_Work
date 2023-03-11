#include <iostream>

void test_open_binary();

void test_open_txt();

using namespace std;



int main() {
    test_open_txt();
    test_open_binary();
    return 0;
}

void test_open_txt() {
    const char *path = "/Users/dev/Documents/GCC_Work/CPP/fopen/doc/佳佳.txt";
    // 以文本的方式写文件
    FILE *fp = fopen(path, "wt");
    if (fp) {
        for (int i = 0; i < 10; ++i) {
            fprintf(fp, "%s%s\n", "12", "34");
        }
        fclose(fp);
        fp = nullptr;
    }

}

void test_open_binary() {
    const char *path = "/Users/dev/Documents/GCC_Work/CPP/fopen/doc/佳佳2.txt";
    FILE *fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 10; ++i) {
            fprintf(fp, "%s%s\n", "12", "34");
        }
        fclose(fp);
        fp = nullptr;
    }


}

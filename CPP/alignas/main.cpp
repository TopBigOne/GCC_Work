#include <iostream>

struct Storage {
    char a;
    int b;
    double c;
    long long d;
};

struct alignas(std::max_align_t) AlignasStorage {
    char a;
    int b;
    double c;
    long long d;
};

typedef struct xAndY {
    int x;
    double y;
} xAndY;

void test_one();

void test_alignof();

void test_alignas();

int main() {
    std::cout << alignof(Storage) << std::endl;
    std::cout << alignof(AlignasStorage) << std::endl;
    test_alignof();
    test_alignas();
    return 0;
}


/**
 * 使用 alignof 查询类型的对齐要求
 */
void test_alignof() {
    puts("test_alignof : alignof");
    std::cout << "Alignment of int    : " << alignof(int) << std::endl;
    std::cout << "Alignment of double : " << alignof(double) << std::endl;
    std::cout << "Alignment of struct : " << alignof(xAndY) << std::endl;

}


/**
 * 使用 alignas 指定变量的对齐要求
 */
void test_alignas() {
    puts("test_alignas : alignas");
    alignas(16) int x;  // 对齐要求为 16 字节的 int 变量
    alignas(double) char buffer[sizeof(double)];  // 对齐要求为 double 类型的缓冲区

     struct alignas(64) S {
        int a;
        double b;
    } ;  // 对齐要求为 64 字节的结构体

    S s;

    std::cout << "Alignment of x      : " << alignof(decltype(x)) << std::endl;
    std::cout << "Alignment of S      : " << alignof(decltype(s)) << std::endl;
    std::cout << "Alignment of S.a    : " << alignof(decltype(s.a)) << std::endl;
    std::cout << "Alignment of S.b    : " << alignof(decltype(s.b)) << std::endl;


}

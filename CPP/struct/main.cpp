#include <iostream>


void test_one();

void test_two();


struct Book {
    char   *name;
    double price;

};

struct Address {
    char *name;
};

struct BookStore {
    Book    book;
    Address address;

};

typedef struct MM {
    char   name[20];  // 20 byte 拆分： 16+4，需要额外 +4 ，组和成8byte
    double score;     // 8 byte
    int    age;       // 4 byte
}MM;

typedef struct NN {
    double score;     // 8 byte
    int    age;       // 4 byte  和 name中的 4 个byte 恰好 凑成 8个byte ，正好对齐
    char   name[20];  // 20 byte  拆分：16+4
}NN;


void test_two() {
    puts("测试结构体 内存;");
    printf("MM : %zd\n", sizeof(MM));
    printf("NN : %zd\n", sizeof(NN));


}

void test_one() {
    puts("测试结构体 声明:");
    Book book{};
    book.name  = "C++讲义";
    book.price = 12.6;

}


int main() {
    test_one();
    test_two();

    return 0;
}

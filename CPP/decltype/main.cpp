#include <iostream>

using namespace std;


template<typename T>
using PtrType = decltype(std::declval<T *>());
struct MyStruct {
    int value;
};


void test_one();

void test_two();

int main() {
    test_one();
    test_two();
    return 0;
}

void test_two() {
    PtrType<MyStruct> ptr = nullptr;

}

void test_one() {
    auto x = 1;
    auto y = 2;
    // 推导出z的类型是int
    decltype(x) z;
    cout << "the s z value is :" << z << endl;
    decltype(10.8) w = 5;



}

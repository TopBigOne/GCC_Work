#include <iostream>

#include <string>

using namespace std;

int max(int x, int y) {
    return x >= y ? x : y;
}

int min(int x, int y) {
    return x >= y ? y : x;
}


auto desc_one = "case 1: 作为指针变量，进行赋值调用";
auto desc_two = "case 2: 作为参数，进行函数回调";

void test_one();

void test_two();


void fun_desc(string desc) {
    cout << desc << endl;
}

int main() {
    test_one();
    test_two();
    return 0;
}

void test_one() {
    fun_desc(desc_one);
    int a = 10;
    int b = 23;
    int (*func_pointer)(int, int);
    int result = -1;
    // 将max函数赋值 函数指针
    func_pointer = max;
    result = func_pointer(a, b);
    cout << "result one : " << result << endl;
    func_pointer = min;
    result = func_pointer(a, b);
    cout << "result two : " << result << endl;
}


/**
 * 指针形式
 * @param a
 * @param b
 * @param p
 */
void func_one(int &a, int &b, int (*p)(int, int)) {
    cout << p(a, b) << endl;
}

/**
 * 引用形式
 * @param a
 * @param b
 * @param p
 */
void func_two(int &a, int &b, int (&p)(int, int)) {
    cout << p(a, b) << endl;
}

void test_two() {
    fun_desc(desc_two);
    int a = 10;
    int b = 20;
    func_one(a, b, max);
    func_two(a, b, min);

}


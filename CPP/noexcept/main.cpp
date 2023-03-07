#include <iostream>

using namespace std;

void func_not_throw() noexcept;

void test_one();

void test_two();

// 开始测试noexcept 带参数的使用方法；
void may_throw();

void may_throw1() noexcept(false);

void no_throw() noexcept;

auto lmay_throw = [] {};
auto lmay_throw1 = []() noexcept(false) {};
auto lno_throw = []() noexcept {};

int main() {

    test_two();
    test_one();
    return 0;
}

void test_two() {

    std::cout << std::boolalpha
              << "Is may_throw() noexcept? " << noexcept(may_throw()) << '\n'
              << "Is may_throw1() noexcept? " << noexcept(may_throw1()) << '\n'
              << "Is no_throw() noexcept? " << noexcept(no_throw()) << '\n'
              << "Is lmay_throw() noexcept? " << noexcept(lmay_throw()) << '\n'
              << "Is lmay_throw1() noexcept? " << noexcept(lmay_throw1()) << '\n'
              << "Is lno_throw() noexcept? " << noexcept(lno_throw()) << '\n';


}

void test_one() {
    try {
        // 直接 terminate，不会被 catch
        func_not_throw();

    } catch (int) {
        cout << "catch int" << endl;
    }

}

void func_not_throw() noexcept {
    cout << "进入 func_not_throw 内部" << endl;
    throw 1;

}

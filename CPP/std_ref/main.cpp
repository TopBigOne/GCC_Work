#include <iostream>
#include <thread>

void foo1(int *x) {
    *x += 10;
}

void test_1() {
    int a = 5;
    int *p_a = &a;
    std::thread t1(foo1, p_a);
    t1.join();
    std::cout << "case 1 : a :" << a << std::endl;
}


void foo2(int &x) {
    x += 11;
}

void test_2() {
    int b = 5;
    std::thread t2(foo2, std::ref(b));
    t2.join();
    std::cout << "case 2 : b :" << b << std::endl;
}



int main() {
    test_1();
    test_2();
    return 0;
}

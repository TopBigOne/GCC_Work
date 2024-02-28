//
// Created by dev on 2023/4/9.
//

// std::unique_ptr 是一种独占的智能指针，它禁止其他智能指针与其共享同一个对象，从而保证代码的安全：

#include <iostream>
#include <memory>

using namespace std;

struct Foo {
    // 我草....结构体，也能有构造函数？？？？
    Foo() {
        cout << "   Foo:Foo()" << endl;
    }

    ~Foo() {
        cout << "   Foo:~Foo()" << endl;
        cout << "-------------------end---------------------------------" << endl;
    }

    void fun_1() {
        cout << "   invoke Foo : fun_1 " << endl;
    }
};

void f(const Foo &) {
    std::cout << "f(const Foo&)" << std::endl;
}

void test_unique_ptr() {
    puts("test unique_ptr");
    unique_ptr<Foo> p1 = make_unique<Foo>();
    if (p1) p1->fun_1();
    {
        unique_ptr<Foo> p2(move(p1));
        f(*p2);// p2 不空，输出；
        if (p2) {
            p2->fun_1();
        }

        // p1 为空, 无输出
        if (p1) {
            p1->fun_1();
        }

        p1 = move(p2);
        if (p2) {
            p2->fun_1();
        }
        std::cout << "  p2 被销毁" << std::endl;
    }

    // p1 不空, 输出
    if (p1 == nullptr) {
        cout << "the p1 is NULL now." << endl;
    } else {
        p1->fun_1();
    }

    //  p1失去了控制权,同时p1指向nullptr
    p1.release();
    if (p1 == nullptr) {
        cout << "the p1 is NULL now." << endl;
    }


    // Foo 的实例会在离开作用域时被销毁

}


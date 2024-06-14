#include <iostream>

#include <memory>

using namespace std;

struct Foo {
    Foo() {
        cout << "Foo 构造函数" << endl;
    }

    ~Foo() {
        cout << "Foo::~Foo" << endl;
    }

    void foo() {
        cout << "invoke foo" << endl;
    }
};

void f(const Foo &) {
    std::cout << "f(const Foo&)" << std::endl;
}

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args &&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int main() {
    std::unique_ptr<Foo> p1(make_unique<Foo>());
    // case 1： 函数调用
    if (p1) {
        p1->foo();
    }

    {
        // case 2： 把p1的所有权转移给p1
        unique_ptr<Foo> p2(move(p1));
        f(*p2);

        if (p2) {
            p2->foo();
        }
        if (p1) {
            p1->foo();
        } else {
            cout << "p1 已经没了所有权 p1 已经是空指针了：， %p1: " << p1 << endl;
        }
        // case 3:  在此给p1 所有权
        p1 = move(p2);
        if (p2) {
            p2->foo();
        } else {
            cout << "p2 已经没了所有权 p2 已经是空指针了：， %p2: " << p2 << endl;
        }
        cout << "%p1 " << p1 << endl;
    }

    return 0;
}

#include <iostream>
#include <memory>

using namespace std;


#include "Sunday.h"

void test_one();

void test_two();

void test_three();

void test_four();

struct MyClass {
    int data;

    MyClass(int value) : data(value) {}
};


int main() {

    test_two();
    test_three();
    test_four();
    return 0;
}


void test_one() {
    // 使用 make_shared 创建 shared_ptr 对象
    std::shared_ptr<MyClass> ptr   = std::make_shared<MyClass>(42);
    // 访问对象成员
    int                      value = ptr->data;
    cout << "value : " << value << endl;

}

void test_two() {
    shared_ptr<int> ptr(new int(42));
    cout << "ptr1 Value : " << *ptr << endl;
    cout << "reference count  : " << ptr.use_count() << endl;
    // 创建另一个 shared_ptr，与前一个共享对象所有权,use_count 会变成3；
    shared_ptr<int> ptr2 = ptr;
    shared_ptr<int> ptr3 = ptr;
    // cout << "reference count  : " << ptr.use_count() << endl;

    ptr2.reset(new int(23));
    cout << "ptr2 Value       : " << *ptr2 << endl;
    cout << "reference count  : " << ptr.use_count() << endl;

    ptr.reset();
    cout << "reference count  : " << ptr.use_count() << endl;

}

void test_three() {
    puts("test_three : test unique_ptr:");
    unique_ptr<Sunday> ptr = unique_ptr<Sunday>();
    ptr->DoSomething();

}

void test_four() {
    puts("test_four: test unique_ptr:");
    unique_ptr<Sunday> ptr(new Sunday);
    ptr->DoSomething();

    unique_ptr<Sunday> ptr2 = std::move(ptr);
    if (ptr == nullptr) {
        puts("   ptr is NULL");
    }
    // 将 ptr2 重置为空指针
    ptr2.reset();

    if (ptr2 == nullptr) {
        puts("   ptr2 is NULL");
    }


}

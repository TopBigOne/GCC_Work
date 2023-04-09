//
// Created by dev on 2023/4/9.
//
#include <iostream>
#include <memory>

using namespace std;

void foo(std::shared_ptr<int> i) {
    //  (*i) : 获取指针的值，然后+1；
    (*i)++;
}

void create_prt() {
    auto pointer = std::make_shared<int>(10);
    foo(pointer);
    cout << "*b_pointer value is : " << *pointer << endl;
}


void get_shared_ptr() {
    cout << "get() : 获取原始指针；" << endl;
    auto pointer  = make_shared<int>(10);
    auto pointer2 = pointer; // 引用 计数+1；
    auto pointer3 = pointer; // 引用 计数+1；
    int  *p       = pointer.get();         // 这样不会增加 引用计数
    std::cout << "b_pointer.use_count() = " << pointer.use_count() << std::endl;   // 3
    std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 3
    std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 3

    // 减少一个指针的计数
    pointer2.reset();

    std::cout << " I had reset pointer2:" << std::endl;
    std::cout << "b_pointer.use_count() = " << pointer.use_count() << std::endl;   // 2
    std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl;  // pointer2 已 reset; 0
    std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 2

    int *temp = new int(100);
    pointer3.reset(temp);//  因为temp 已近被 智能指针 wrap，不需要手动delete
    std::cout << "now the pointer3 value is : " << *pointer3 << std::endl;  // 100
    std::cout << "now the b_pointer value  is  : " << *pointer << std::endl;  // 10




}
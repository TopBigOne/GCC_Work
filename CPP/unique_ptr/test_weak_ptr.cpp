//
// Created by dev on 2023/4/9.
//

#include <iostream>
#include <memory>

using namespace std;
struct A;
struct B;

struct A {
    std::shared_ptr<B> b_pointer;

    ~A() {
        std::cout << "A 被销毁" << std::endl;
    }
};

struct B {
    std::weak_ptr<A> a_pointer;

    ~B() {
        std::cout << "B 被销毁" << std::endl;
    }
};

/**
 * share_ptr 的环形引用
 * @return
 */
void test_share_cycle_reference() {
    cout << "测试 share_ptr 的环形引用" << endl;
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();

    a->b_pointer = b;
    b->a_pointer = a;
    cout << "a use_count is : " << a.use_count() << endl;
    cout << "b use_count is : " << b.use_count() << endl;

}


/**
 * share_ptr 的环形引用
 * @return
 */
void test_share_cycle_reference_2() {
    cout << "测试 weak_ptr 在环形引用的使用：" << endl;
    auto a = std::make_shared<A>();
    a->b_pointer = make_shared<B>();

    auto a_weak_prt = weak_ptr<A>(a);


    cout << "a use_count is          : " << a.use_count() << endl;
    cout << "a_weak_prt use_count is : " << a_weak_prt.use_count() << endl;
    cout << "a_weak_prt  is expired : " << a_weak_prt.expired() << endl;

}

int test_weak_ptr_lock() {
    puts("测试：std::weak_ptr 类提供的 lock() 方法来 获取管理所监测资源的 shared_ptr 对象");
    shared_ptr<int> sp1;
    sp1 = std::make_shared<int>(520); // sp1 引用+ 1
    cout << " sp1 use_count  is : " << sp1.use_count() << endl;


    shared_ptr<int> sp2;

    weak_ptr<int> wp;

    wp = sp1; //  sp1 引用+ 1
    //
    auto unknown_sp2 = wp.lock();
    cout << "unknown_sp2: " << unknown_sp2.use_count() << endl;
    sp1.reset();
    cout << "use_count after sp1 invoke reset , wp use count is :  " << wp.use_count() << endl;

    sp1 = wp.lock();
    cout << "use_count: " << wp.use_count() << endl;

    cout << "*sp1: " << *sp1 << endl;
    cout << "*sp2: " << *sp2 << endl;

    return 0;
}



#include <iostream>
#include <string>
#include <utility>  // for std::move
#include "MyObject.h"
#include "MoveDemo.h"

using namespace std;

void test1();
void test2();
void test3();
void test4();
void runOriginalTests();

void testCore();

int main() {
    testCore();

    return 0;
}

void testCore() {
    cout << "=== C++ std::move 学习项目 ===" << endl;
    cout << "本项目基于 README.md 中的理论知识，提供完整的可运行演示" << endl;
    cout << "\n选择演示模式:" << endl;
    cout << "1. 完整交互式演示 (基于README.md的所有概念)" << endl;
    cout << "2. 原有测试函数演示 (test1-test4)" << endl;
    cout << "请输入选择 (1, 2): ";

    char choice;
    cin >> choice;
    cin.ignore(); // 清除输入缓冲区

    switch (choice) {
        case '1':
            // 运行完整的演示程序
            MoveShowcase::runCompleteDemo();
            break;

        case '2':
            // 运行原有的测试函数
            runOriginalTests();
            break;
        default:
            cout << "无效选择，运行原有测试..." << endl;
            runOriginalTests();
            break;
    }

}

void runOriginalTests() {
    cout << "\n=== 原有测试函数演示 ===" << endl;

    cout << "\n--- test1: 基本移动构造 ---" << endl;
    test1();

    cout << "\n--- test2: 字符串和基本类型移动 ---" << endl;
    test2();

    cout << "\n--- test3: 移动构造详细演示 ---" << endl;
    test3();

    cout << "\n--- test4: 移动赋值演示 ---" << endl;
    test4();
}

void test1() {
    MyObject obj1;
    // step 1：创建一个MyObject对象
    obj1.setName("筱雅");
    obj1.addNameList();
    obj1.printObj();
    // step 2:o1 move o2
    // 使用std::move()将obj1的所有权转移到obj2
    MyObject obj2 = std::move(obj1);
    cout << "看看obj1的值：" << endl;
    obj1.printObj();
    obj1.setName("chen xiao ya");
    cout << "给obj1 的属性再次赋值，再看看 obj1 的值：" << endl;
    obj1.printObj();
    cout << "看看obj2的值：" << endl;
    obj2.printObj();
}

void test2() {
    std::cout << "test2" << std::endl;
    std::string a = "hello world";
    //  这里是移动，不是拷贝！
    // b: hello world
    std::string b = std::move(a);
    //  a: （内容未定义，通常为空字符串）
    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    int d = std::move(2);
    std::cout << "d: " << d << std::endl;
}

void test3() {
    std::cout << "测试移动构造" << std::endl;
    MyObject obj1;
    obj1.setName("123");
    obj1.addNameList();
    std::cout << "打印obj1" << std::endl;
    obj1.printObj();
    auto obj2 = std::move(obj1);
    std::cout << "打印obj2" << std::endl;
    obj2.printObj();
    std::cout << "打印obj1" << std::endl;
    obj1.printObj();
}

void test4() {
    std::cout << "测试移动赋值" << std::endl;
    MyObject obj1;
    obj1.setName("234");
    obj1.addNameList();
    std::cout << "打印obj1" << std::endl;
    obj1.printObj();

    MyObject obj2;
    obj2.setName("郭源潮");
    obj2.addNameList();
    std::cout << "打印obj2" << std::endl;
    obj2.printObj();
    std::cout << "开始赋值" << std::endl;
    obj1 = std::move(obj2);
    std::cout << "赋值结果" << std::endl;

    std::cout << "打印obj1" << std::endl;
    obj1.printObj();

    // NOTE: obj2 里的变量现在已经变成空的了
    std::cout << "打印obj2" << std::endl;
    obj2.printObj();
}

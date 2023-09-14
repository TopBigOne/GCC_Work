#include <iostream>

#include "add.h"
#include "MyStack.h"
#include "TemplateSpecialization.h"

using namespace std;

void test_one();

void test_two();

void test_three();

int main() {
    test_one();
    test_two();
    test_three();
    return 0;
}


void test_one() {
    cout << "case 1: 测试模板函数：" << endl;
    int a = Add(3, 5);
    cout << "Add result is :" << a << endl;
}


void test_two() {
    cout << "case 2: 测试模板类：" << endl;
    MyStack<int> myStack;
    myStack.push(2);
    myStack.push(6);
    myStack.push(45);

    int value = myStack.pop();
    cout << "pop value  is :" << value << endl;
}


void test_three() {
    cout << "case 3: 模板特化  ：" << endl;
    TemplateSpecialization templateSpecialization;
    templateSpecialization.print(1200);
    templateSpecialization.print(23.5);
    templateSpecialization.print(std::string("dfd"));
}


#include <iostream>

#include "Person.h"

void testTwo();

void testOne();

using namespace std;

// C++ new对象与直接声明对象区别:https://blog.csdn.net/weixin_44556968/article/details/105585210
int main() {
    testOne();
    testTwo();

    return 0;
}

void testOne() {
    puts("case : 指针和 普通对象的用法");
    Person personOne;
    personOne.age = 10;
    personOne.name = "小丽";
    cout << "person one info:" << endl;
    cout << "                 age: " << personOne.age << endl;
    cout << "                 name: " << personOne.name << endl;
    auto personTwo = new Person();
    personTwo->age = 100;
    personTwo->name = "小艳";
    cout << "person two info:" << endl;
    cout << "                 age: " << personTwo->age << endl;
    cout << "                 name: " << personTwo->name << endl;
    delete personTwo;

    char charOne[200] = {0};

}

void testTwo() {
    puts("case 2: 怎么获取指针的数据");
    int a = 15;
    int *p = &a;
    printf("    a   : %d\n", a);
    printf("    *p  : %d\n", *p);
}

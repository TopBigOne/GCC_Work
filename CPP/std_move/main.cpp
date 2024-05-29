#include <iostream>
#include "MyObject.h"

void testOne();

void testTwo();

int main() {
    testTwo();

    // testOne();
    return 0;
}

MyObject obj1;

void testOne() {
    // step 1：创建一个MyObject对象


    string name1("筱雅");
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

void testTwo() {
    double a = 20.5;
    double b = 36;
    double c = 23.5;
    double d = 34.2;
    // 实际107
    double r = a + b + c + d;
    cout << "粗铜：" << r << " kg" << endl;
    double result = r * 63;
    cout << "粗铜--- result:：" << r * 63 << " 元" << endl;
    // 9 公斤的细线，
    cout << "RRRR---result2 :9*40 " << 360 << endl;
    cout << "RRRR---result2 :：" << r * 63 + 360 << endl;

    cout << "-----------------------------" << endl;
    cout << "-----------------------------" << endl;
    cout << "-----------------------------" << endl;
    double real_1 = 108 * 70;
    double real_3 = 7.5 * 70 - 360;
    cout << "real_1--- : 108 * 70       :" << real_1 << endl;
    cout << "real_2--- : real_1 -result :" << real_1 - result << endl;
    cout << "real_3--- : 细铜线      7.5 * 70 - 360  :" << real_3 << endl;
    cout << "real_4--- : all  real_2+ real_3 :" << real_1 - result + real_3 << endl;
}

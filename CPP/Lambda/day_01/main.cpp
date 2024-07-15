#include <iostream>

using namespace std;


/**
 * 最简单的lamda
 */
auto ifn = []() {
    cout << "first day lambda" << endl;
};


auto                     ifnParam = [](int val) {
    cout << "val : " << val << endl;
};

// <void(int)> 表示该函数对象接受一个整数参数并返回 void。
std::function<void(int)> ifnFunc  = [](int val) {
    cout << "val : " << val << endl;
};

// std::function<int(int)>: 这是一个函数对象类型模板，用于定义一个可调用对象的类型。在这里，<int(int)> 表示该函数对象接受一个整数参数并返回一个整数
std::function<int(int)> ifnFunc2  = [](int val) {
    cout << "val : " << val << endl;
    return val+20;
};



void test() {

    ifn();
    ifnParam(10);
    ifnFunc(20);
    ifnFunc2(21);

}

int main() {
    test();
    return 0;
}

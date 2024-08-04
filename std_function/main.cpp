#include <iostream>

#include <functional>

using namespace std;

void sayHello() {
    cout << "Hello,world " << endl;
}

void sayHello2(int v) {
    cout << "Hello,world : " << v << endl;
}

void lambdaFun(int tA, int tB) {
    function<int(int, int)> myAdd = [](int a, int b) {
        return a + b;
    };
    cout << tA << "+" << tA << "=" << myAdd(tA, tB) << endl;
}

void process(function<int(int, int)> func, int a, int b) {
    cout << " Result " << func(a, b) << endl;

}

void case_1();

void case_2();

void case_3();

void case_4();


int main() {
    case_1();
    case_2();
    case_3();
    case_4();
    return 0;
}

void case_1() {
    cout << "case 1: 使用 std::function 封装函数           :" << endl;
    function<void()>    func  = sayHello;
    func();
    function<void(int)> func2 = sayHello2;
    func2(2);
}

void case_2() {
    cout << "case 2: 使用 std::function 封装 lambda 表达式 :" << endl;
    lambdaFun(23, 45);

}

void case_3() {
    cout << "case 3: 使用 std::function 作为参数传递 :" << endl;
    function<int(int, int)> multiply = [](int x, int y) {
        return x * y;
    };
    process(multiply, 3, 4);
}


void performOperation(int x, int y, function<void(int)> callback) {
    int result = x * y;
    callback(result);

}

void case_4() {
    cout << "case 4: 使用 std::function 实现回调函数 :" << endl;
    performOperation(5, 7, [](int result) {
        cout << "Result: " << result << endl;
    });
}






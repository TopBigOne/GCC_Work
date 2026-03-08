#include <iostream>
#include <string>

#include <vector>

using namespace std;


class Calculator {
public:
    void multiply(int a, int b) {
        cout << "Product :" << a * b << endl;
    }
};

void test_one();

void test_two();

void test_three();

void test_four();





void test_one() {
    auto operation = [](int a, int b, string op) -> double {
        if (op == "sum") {
            return a + b;
        } else {
            return (a + b) / 2.0;
        }
    };
    int  num1      = 1;
    int  num2      = 2;
    auto sum       = operation(num1, num2, "sum");
    cout << "sum : " << sum << endl;

    auto average = operation(num1, num2, "avg");
    cout << "avg: " << average << endl;
}


void test_two() {
    vector<int> vec{1, 2, 3, 4, 5};
    for_each(vec.begin(), vec.end(),
             [](int &a) {// 注意这里的传值是引用
                 a *= 2;
             });
    for_each(vec.begin(), vec.end(), [](int a) {
        cout << "new value : " << a << endl;
    });
}


void printSum(int a, int b) {
    cout << "Sum :" << a + b << endl;
}
void test_three() {
    puts("case 1: 绑定全局函数或静态成员函数：");
    auto func = bind(printSum, 10, 35);
    func();
}

void test_four() {
    puts("case 2: 绑定成员函数：");
    Calculator calculator;
    auto       func = std::bind(&Calculator::multiply, &calculator, 5, 3);
    func();
}


int add(int a, int b) {
    return a + b;
}

/**
 * 参数默认值设置
 */
void test_five() {
    puts("case 3: 延迟调用：");
    // 在这里我们只提供了一个参数，第二个参数会在调用时传递
    // std::placeholders::_1 就是一个临时占位符；
    auto func   = std::bind(add, 10, std::placeholders::_1);
    int  result = func(20);
    cout << " result :" << result << endl;

}



int main() {
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    return 0;
}


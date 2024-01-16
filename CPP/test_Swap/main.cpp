#include <iostream>

using namespace std;


void niubilityDivide();

void testValuePassing();

void testReferencePassing();

void testPointPassing();

void niubilityDivide() {
    cout
            << "******************************************************************************************************************"
            << endl;
    cout << endl;
}

void swap(int num1, int num2) {
    cout << "交换前：" << endl;
    cout << "   num1 = " << num1 << endl;
    cout << "   num2 = " << num2 << endl;

    int temp = num1;
    num1 = num2;
    num2 = temp;

    cout << "交换后：" << endl;
    cout << "   num1 = " << num1 << endl;
    cout << "   num2 = " << num2 << endl;

    //return ; 当函数声明时候，不需要返回值，可以不写return
}


void swap2(int &num1, int &num2) {
    cout << "交换前：" << endl;
    cout << "   num1 = " << num1 << endl;
    cout << "   num2 = " << num2 << endl;

    int temp = num1;
    num1 = num2;
    num2 = temp;

    cout << "交换后：" << endl;
    cout << "   num1 = " << num1 << endl;
    cout << "   num2 = " << num2 << endl;

    //return ; 当函数声明时候，不需要返回值，可以不写return
}


void swap3(int *num1, int *num2) {
    cout << "交换前：" << endl;
    cout << "   num1 = " << *num1 << endl;
    cout << "   num2 = " << *num2 << endl;

    int *temp = num1;
    *num1 = *num2;
    *num2 = *temp;

    cout << "交换后：" << endl;
    cout << "   num1 = " << *num1 << endl;
    cout << "   num2 = " << *num2 << endl;

    //return ; 当函数声明时候，不需要返回值，可以不写return
}


void testValuePassing() {
    cout << "测试值传递" << endl;
    int a = 10;
    int b = 20;

    swap(a, b);

    cout << "-----------------------------------------------------------" << endl;
    cout << "   函数中的 a = " << a << endl;
    cout << "   函数中的 b = " << b << endl;
    cout << "-----------------------------------------------------------" << endl;
    cout << "    main 的值，依旧灭变" << endl;
    niubilityDivide();


}

void testReferencePassing() {
    cout << "测试引用传递" << endl;
    int a = 10;
    int b = 20;

    swap2(a, b);

    cout << "-----------------------------------------------------------" << endl;
    cout << "   函数中的 a = " << a << endl;
    cout << "   函数中的 b = " << b << endl;
    cout << "-----------------------------------------------------------" << endl;
    cout << "    a,b 的值发生交换" << endl;
    niubilityDivide();

}

void testPointPassing() {
    cout << "测试指针传递" << endl;
    int a = 10;
    int b = 20;

    swap2(a, b);

    cout << "-----------------------------------------------------------" << endl;
    cout << "   函数中的 a = " << a << endl;
    cout << "   函数中的 b = " << b << endl;
    cout << "-----------------------------------------------------------" << endl;
    cout << "    a,b 的值发生交换" << endl;
    niubilityDivide();

}

int main() {
    testValuePassing();
    testReferencePassing();
    testPointPassing();


    return 0;
}

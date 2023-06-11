#include <iostream>

void test1();

void test2();

void test3();

using namespace std;


int main() {

    test1();
    test2();
    test3();
    return 0;
}

void test3() {
    puts("用指针引用数组元素");
    int a[] = {5, 7, 2, 6, 13};
    int *p  = &a[0];
    int *t  = a;
    cout << "*p : " << *p << endl;

    for (int i = 0; i < 5; ++i){
        // 指针的移动
        cout << "*t : " << *(t++) << endl;
    }



}


/**
 * 数组指针
 */
void test1() {
    puts("数组指针:");
    double array[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
    // todo 直接存的？
    double (*p)[5] = &array;

    for (int i = 0; i < 5; ++i) {
        cout << (*p)[i] << endl;
    }
}

/**
 * 指针数组
 */
void test2() {
    puts("指针数组");
    double array[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
    // 存的地址；
    double *p[5];

    for (int i = 0; i < 5; ++i) {
        p[i] = &array[i];
    }
    cout << (*p)[2] << endl;

}

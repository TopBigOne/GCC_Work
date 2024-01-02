#include <iostream>

#include "TestMaxElement.h"

void testOne();

void testTwo();

void testThree();

using namespace std;

template<class T>
void Swap(T &x, T &y) {
    T temp = x;
    x = y;
    y = temp;
}


void testOne() {
    puts("testOne : ");
    int n = 1;
    int m = 2;
    Swap(n, m);
    double f = 1.2;
    double g = 2.3;
    Swap(f, g);

}

template<class T>
T Inc(int n) {
    return n + 1;
}


void testTwo() {
    puts("testTwo : ");
    // 告诉编译器，返回的结果是double类型；
    double result = Inc<double>(4) / 1.34;
    cout << "result : " << result << endl;
}


void testThree() {
    puts("testThree : ");
    int            a[5] = {1, 5, 2, 3, 4};
    TestMaxElement f[4] = {TestMaxElement(8, 6),
                           TestMaxElement(-8, 4),
                           TestMaxElement(3, 2),
                           TestMaxElement(5, 6)};
    cout << MaxElement(a, 5) << endl;
    cout << MaxElement(f, 4) << endl;


}


/**
 * 类型参数的数量没有实际限制。 以逗号分隔多个参数：
 * @tparam T
 * @tparam U
 * @tparam V
 */
template <typename T, typename U, typename V> class Foo{

    };

/**
 * 在此上下文中，关键字 class 等效于 typename。 可以将前面的示例表示为：
 * @tparam T
 * @tparam U
 * @tparam V
 */
template <class T, class U, class V> class Foo1{};





int main() {
    testOne();
    testTwo();
    testThree();


    return 0;
}

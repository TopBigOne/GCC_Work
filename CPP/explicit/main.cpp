#include <iostream>

#include "NormalClass.h"
#include "ExplicitClass.h"

using namespace std;

void testOne(NormalClass obj);

void testTwo(ExplicitClass obj);


void testOne(NormalClass obj) {

    cout << "testOne" << endl;
    cout << "   obj.data : " << obj.data << endl;
}

void testTwo(ExplicitClass obj) {
    cout << "testTwo" << endl;
    cout << "   obj.data : " << obj.data << endl;

}

int main() {
    // 隐式类型转换
    testOne(42);
    // 必须显式创建 ExplicitClass 对象
    testTwo(ExplicitClass(43));
    return 0;
}

#include <iostream>

using namespace std;

void my_print(char *temp);


void my_print(char *temp) {
    cout << "content is : " << temp << std::endl;
}


void test_1();

void test_2();

void test_3();

void test_2() {
    int *p = new int;
    *p = 23446;

    cout << "*p is : " << *p << endl;
    *p = 0;
    cout << "*p is : " << *p << endl;
}


void test_1() {
    char temp[] = "123";
    my_print(temp);
    strcpy(temp, "456");
    my_print(temp);
}


void test_3() {
    allocator<int> allocator;
    // step 1:分配一块内存，可以容纳两个int
    int            *arr = allocator.allocate(2);
    // step 2: 构造对象
    allocator.construct(arr, 42);
    allocator.construct(arr + 1, 88);

    // step 3: 访问对象
    cout << arr[0] << endl;
    cout << arr[1] << endl;
    // step 4: 销毁对象
    allocator.destroy(arr);
    allocator.destroy(arr + 1);

    // step 5: 释放内存
    allocator.deallocate(arr, 2);

}


int main() {
    test_1();
    test_2();
    test_3();
    return 0;
}

#include <iostream>

void test_one();

using namespace std;

int main() {
    test_one();
    return 0;
}

void test_one() {
    constexpr int num = 1 + 2 + 3;
    // 数组必须是一个常量
    int url[num] = {1, 2, 3, 4, 5, 6};
    cout << url[1] << endl;

}

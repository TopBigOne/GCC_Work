#include <iostream>

#include <vector>

using namespace std;

vector<int> v = {1, 2, 3, 4, 5, 6};

void test1();

void test2();

int main() {
    test1();
    test2();
    return 0;
}

void test1() {
    puts("find () :线性查找");
    auto it = find(v.begin(), v.end(), 5);
    if (it == v.end()) {
        std::cout << "  find(): Element not found." << std::endl;
    } else {
        std::cout << "  find(): Element found." << std::endl;
    }
}

void test2() {
    puts("binary_search () : 二分搜索算法");
    bool found = binary_search(v.begin(), v.end(), 3);

    if (!found) {
        std::cout << "  binary_search(): Element not found." << std::endl;
    } else {
        std::cout << "  binary_search(): Element found." << std::endl;
    }
}

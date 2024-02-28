#include <iostream>
#include <forward_list>

using namespace std;

void test_1();

int main() {
    test_1();

    return 0;
}

void test_1() {
    std::forward_list<int> f_list = {10, 20, 30};
    f_list.push_front(40);
    for (const auto &item: f_list) {
        cout << "item is :" << item << endl;

    }

    f_list.pop_front();

    cout << "-------------" << endl;
    for (const auto &item: f_list) {
        cout << "item is :" << item << endl;

    }


}

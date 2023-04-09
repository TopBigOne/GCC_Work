#include <iostream>
#include <string>

#include <vector>

using namespace std;

void test_one();

void test_two();

int main() {
    test_one();
    test_two();
    return 0;
}


void test_one() {
    auto operation = [](int a, int b, string op) -> double {
        if (op == "sum") {
            return a + b;
        } else {
            return (a + b) / 2.0;
        }
    };
    int num1 = 1;
    int num2 = 2;
    auto sum = operation(num1, num2, "sum");
    cout << "sum : " << sum << endl;

    auto average = operation(num1, num2, "avg");
    cout << "avg: " << average << endl;
}


void test_two() {
    vector<int> vec{1, 2, 3, 4, 5};
    for_each(vec.begin(), vec.end(),
             [](int &a) {
                 a *= 2;
             });
    for_each(vec.begin(), vec.end(), [](int a) {
        cout << "new value : " << a << endl;
    });
}

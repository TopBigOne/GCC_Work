#include <iostream>

void test_one();

using namespace std;

double division(int a, int b) {
    if (b == 0) {
        throw "Division by zero condition";
    }
    return (a / b);
}

int main() {
    test_one();
    return 0;
}

void test_one() {
    int x = 50;
    int y = 0;
    double z = 0;
    try {
        z = division(x, y);
    } catch (const char *err_msg) {
        cerr << err_msg << endl;
    }

}

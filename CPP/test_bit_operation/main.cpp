#include <iostream>

using namespace std;

int LEFT_CHANNEL    = 0x4;
int RIGHT_CHANNEL   = 0x8;
int DEFAULT_CHANNEL = LEFT_CHANNEL | RIGHT_CHANNEL;

void test_one();

int main() {
    test_one();
    return 0;
}

void test_one() {
    cout << "DEFAULT_CHANNEL :" << DEFAULT_CHANNEL << endl;
}

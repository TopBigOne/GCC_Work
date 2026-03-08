#include <iostream>

using namespace std;

#include "BaseLambda.h"
#include "CaptureList.h"


void test() {
    BaseLambda baseLambda;
    baseLambda.test_one();
}

void testCaptureList();

void testCaptureList() {
    CaptureList captureList;
    captureList.test_one();
    captureList.test_two();
    captureList.test_three();
    captureList.test_Four();
    captureList.test_Five();
    captureList.test_Six();
    captureList.test_Seven();
    captureList.test_eight();
    captureList.test_nine();

}

int main() {
    test();
    testCaptureList();
    return 0;
}

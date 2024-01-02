//
// Created by DEV on 12/29/23.
//

#ifndef TEMPLATE_DEMO_1_TESTMAXELEMENT_H
#define TEMPLATE_DEMO_1_TESTMAXELEMENT_H

using namespace std;

#include <iostream>

template<class T>
T MaxElement(T a[], int size) {
    T        tempMax = a[0];
    for (int i       = 1; i < size; i++) {
        if (tempMax < a[i]) {
            tempMax = a[i];
        }

    }
    return tempMax;

}


class TestMaxElement {
public:

    TestMaxElement(int n, int d) : numerator(n), denominator(d) {}

    bool operator<(const TestMaxElement &f) const;

    bool operator==(const TestMaxElement &f) const;

    friend ostream &operator<<(ostream &o, const TestMaxElement &f);


private:
    int numerator;   //分子
    int denominator; //分母


};


#endif //TEMPLATE_DEMO_1_TESTMAXELEMENT_H

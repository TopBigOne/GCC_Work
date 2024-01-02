//
// Created by DEV on 12/29/23.
//

#include "TestMaxElement.h"

bool TestMaxElement::operator<(const TestMaxElement &f) const {
    if (denominator * f.denominator > 0) {
        return numerator * f.denominator < denominator * f.numerator;
    } else {
        return numerator * f.denominator > denominator * f.numerator;
    }


}

bool TestMaxElement::operator==(const TestMaxElement &f) const {
    return numerator * f.denominator == denominator * f.numerator;
}

ostream &operator<<(ostream &o, const TestMaxElement &f) {
    o << f.numerator << "/" << f.denominator; //输出"分子/分母" 形式
    return o;
}



//
// Created by DEV on 2/19/24.
//

#ifndef EXPLICIT_EXPLICITCLASS_H
#define EXPLICIT_EXPLICITCLASS_H


class ExplicitClass {
public:
    // 使用 explicit 修饰构造函数
    explicit ExplicitClass(int value) : data(value) {}


public:
    int data;
};


#endif //EXPLICIT_EXPLICITCLASS_H

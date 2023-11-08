//
// Created by DEV on 11/8/23.
//

#ifndef ABSTRACTCLASS_ABSTRACTCLASS_H
#define ABSTRACTCLASS_ABSTRACTCLASS_H

#include <iostream>

using namespace std;

class AbstractClass {
public:
    void templateMethod();

    virtual void commonOperation() = 0;

    virtual void hookMethod() = 0;

    virtual void anotherCommonOperation() = 0;


};


#endif //ABSTRACTCLASS_ABSTRACTCLASS_H

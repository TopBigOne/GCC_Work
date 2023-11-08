//
// Created by DEV on 11/8/23.
//

#ifndef ABSTRACTCLASS_CONCRETECLASS_H
#define ABSTRACTCLASS_CONCRETECLASS_H

#include "AbstractClass.h"

class ConcreteClass : public AbstractClass {
public:
    void commonOperation() override;
    void hookMethod() override;
    void anotherCommonOperation() override;

};


#endif //ABSTRACTCLASS_CONCRETECLASS_H

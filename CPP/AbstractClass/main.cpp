#include <iostream>
#include "AbstractClass.h"
#include "ConcreteClass.h"

void testOne();

int main() {
    testOne();
    return 0;
}

void testOne() {
    auto *abstractClass = new ConcreteClass();
    abstractClass->templateMethod();
    delete abstractClass;

}

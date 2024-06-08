//
// Created by dev on 2024/6/8.
//

#ifndef MEMORY_OF_CLASS_B_H
#define MEMORY_OF_CLASS_B_H

#include "A.h"

/**
 *  4+4+8 = 16 byte
 */
class B : public A {

public:
    //alignas(8)
    // 4 byte
    int b;

};


#endif //MEMORY_OF_CLASS_B_H

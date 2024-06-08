//
// Created by dev on 2024/6/8.
//

#ifndef MEMORY_OF_CLASS_C_H
#define MEMORY_OF_CLASS_C_H

#include "B_1.h"
#include "A.h"

/**
 * A: 16 Byte
 * B_1: 12 Byte
 *  int : 4 byte
 *  sizeof(C) = 16+12+4 = 32byte
 */
class C : public A, public B_1 {

public:
    //alignas(8)
    // 4 byte
    int c;

};


#endif //MEMORY_OF_CLASS_C_H

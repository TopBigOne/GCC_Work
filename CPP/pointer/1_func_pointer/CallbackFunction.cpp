//
// Created by DEV on 11/28/23.
//

#include "CallbackFunction.h"


/**
 * callback
 * @param p
 * @param a
 * @param b
 * @return
 */
int CallbackFunction::add(CallbackFunction::callback p, int a, int b) {
    return static_cast<int>((*p)(a, b));
}


int CallbackFunction::sum(int a, int b) {
    return a+b;
}

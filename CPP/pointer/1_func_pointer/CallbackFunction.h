//
// Created by DEV on 11/28/23.
//

#ifndef INC_1_FUNC_POINTER_CALLBACKFUNCTION_H
#define INC_1_FUNC_POINTER_CALLBACKFUNCTION_H


class CallbackFunction {


public:
    typedef int *(callback)(int, int);

public:
    int add(callback p, int a, int b);

    int sum(int a, int b);


};


#endif //INC_1_FUNC_POINTER_CALLBACKFUNCTION_H

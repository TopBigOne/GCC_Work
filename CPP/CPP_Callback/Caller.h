//
// Created by dev on 2023/7/12.
//

#ifndef CPP_CALLBACK_CALLER_H
#define CPP_CALLBACK_CALLER_H

#include "Callback.h"

class Caller {
public:
    void setCallback(Callback *cb);

    void doSomething();


private:
    Callback *m_cb;

};


#endif //CPP_CALLBACK_CALLER_H

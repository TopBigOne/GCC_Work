//
// Created by dev on 2023/7/12.
//

#ifndef CPP_CALLBACK_RECEIVER_H
#define CPP_CALLBACK_RECEIVER_H

#include "Callback.h"
class Receiver : public Callback{
public:
    void onEvent(int data) override;

};


#endif //CPP_CALLBACK_RECEIVER_H

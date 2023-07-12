//
// Created by dev on 2023/7/12.
//

#include "Caller.h"

void Caller::setCallback(Callback *cb) {
    m_cb = cb;
}

void Caller::doSomething() {
    int data = 200;
    if (m_cb) {
        m_cb->onEvent(data);
    }

}

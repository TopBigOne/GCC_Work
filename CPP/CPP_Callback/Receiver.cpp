//
// Created by dev on 2023/7/12.
//

#include "Receiver.h"

#include <iostream>

using namespace std;

void Receiver::onEvent(int data) {
    cout << "Received data : " << data << endl;
}

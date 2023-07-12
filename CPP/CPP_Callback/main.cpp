#include <iostream>

#include "Caller.h"
#include "Receiver.h"

int main() {
    Caller   caller{};
    Receiver receiver;
    caller.setCallback(&receiver);
    caller.doSomething();
    return 0;
}

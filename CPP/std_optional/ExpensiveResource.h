//
// Created by DEV on 2026/3/9.
//

#ifndef STD_OPTIONAL_EXPENSIVERESOURCE_H
#define STD_OPTIONAL_EXPENSIVERESOURCE_H

#include <iostream>

class ExpensiveResource {
public:
    ExpensiveResource() {
        std::cout << "Resource initialized\n";
    }
    ~ExpensiveResource() {
        std::cout << "Resource destroyed\n";
    }
    void use() {
        std::cout << "Resource using\n";
    }
};


#endif //STD_OPTIONAL_EXPENSIVERESOURCE_H
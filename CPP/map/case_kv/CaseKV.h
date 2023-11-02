//
// Created by DEV on 11/2/23.
//

#ifndef MAP_CASEKV_H
#define MAP_CASEKV_H

#include <map>
#include <iostream>

using namespace std;

class CaseKV {

    struct MyStringKey {
        const char *value;

        bool operator<(const MyStringKey &other) const {
            return std::strcmp(value, other.value) < 0;
        }
    };


public:
    void testOne();

    void testTwo();


public:
    map<char *, char *> mMap;

};


#endif //MAP_CASEKV_H

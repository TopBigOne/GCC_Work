//
// Created by dev on 2023/1/18.
//

#ifndef VIRTUAL_PERSON_H
#define VIRTUAL_PERSON_H
using namespace std;

#include <iostream>


class Person {
public:
    Person() {
        cout << "Person构造" << endl;
    }

    ~Person() {
        cout << "Person 析构" << endl;

    }

};


#endif //VIRTUAL_PERSON_H

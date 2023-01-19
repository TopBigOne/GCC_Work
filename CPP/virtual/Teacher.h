//
// Created by dev on 2023/1/18.
//

#ifndef VIRTUAL_TEACHER_H
#define VIRTUAL_TEACHER_H

#include "Person.h"

using namespace std;


class Teacher : virtual public Person {
public:
    Teacher() { cout << "Teacher构造" << endl; }

    ~Teacher() { cout << "Teacher析构" << endl; }

};


#endif //VIRTUAL_TEACHER_H

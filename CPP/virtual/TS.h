//
// Created by dev on 2023/1/18.
//

#ifndef VIRTUAL_TS_H
#define VIRTUAL_TS_H

#include "Teacher.h"
#include "Student.h"

using namespace std;

class TS : public Teacher, public Student {
public:
    TS() { cout << "TS 构造" << endl; }

    ~TS() { cout << "TS 析构" << endl; }

};


#endif //VIRTUAL_TS_H

//
// Created by dev on 2023/1/18.
//

#ifndef VIRTUAL_STUDENT_H
#define VIRTUAL_STUDENT_H

#include "Person.h"

using namespace std;

class Student : virtual public Person {
public:
    Student() { cout << "Student 构造" << endl; }

    ~Student() { cout << "Student 析构" << endl; }
};


#endif //VIRTUAL_STUDENT_H

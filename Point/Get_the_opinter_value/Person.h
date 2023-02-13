//
// Created by dev on 2022/8/9.
//

#ifndef POINT_PERSON_H
#define POINT_PERSON_H

#include <string>

using namespace std;


class Person {
public:
    int age;
    std::string name;
public:
    int getAge();

    string getName();

};


#endif //POINT_PERSON_H

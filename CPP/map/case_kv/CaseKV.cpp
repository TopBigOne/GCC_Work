//
// Created by DEV on 11/2/23.
//

#include "CaseKV.h"

void CaseKV::testOne() {


    mMap.operator[]("1") = "a";
    mMap.operator[]("2") = "b";
    mMap.operator[]("1") = "c";
    mMap.operator[]("1") = "c广亚";


    for (const auto &item: mMap) {
        cout << "key  " << item.first << " value : " << item.second << endl;
    }

    char *result_one = mMap["3"];
    if (result_one == nullptr) {
        cout << "result_one is null" << endl;
    }

    // at ,当key不存在的时候，会crash
    char *result_two = mMap.at("4");


    if (result_two == nullptr) {
        cout << "result_two is null" << endl;
    }


}

void CaseKV::testTwo() {

    int  size = 10;
    char *m1  = new char[size];
    strcpy(m1, "qaz");

    char *m2 = new char[size];
    strcpy(m2, "qaz");

    mMap.operator[](m1) = "123456";
    mMap.operator[](m2) = "123456";

    cout << "map size is : " << mMap.size() << endl;


}

void CaseKV::testThree() {
    map<MyStringKey, const char *> mDeMap;

    int  size = 10;
    char *m1  = new char[size];
    strcpy(m1, "qaz");

    char *m2 = new char[size];
    strcpy(m2, "qaz");

    mDeMap.operator[]({m1}) = "123456";
    mDeMap.operator[]({m2}) = nullptr;

    cout << "map size is : " << mDeMap.size() << endl;
    int a = 10;

}

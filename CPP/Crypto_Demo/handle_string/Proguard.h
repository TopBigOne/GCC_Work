//
// Created by DEV on 10/27/23.
//

#ifndef CRYPTO_DEMO_PROGUARD_H
#define CRYPTO_DEMO_PROGUARD_H

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <string>
#include <map>

#include <fstream>
#include <regex>
#include <iostream>

using namespace std;


class Proguard {


public:

    Proguard(string &key);

    ~Proguard();

    void testCaseOne(const string &plaintext, const string &key);

    string encrypt(const string &plaintext, const string &key);

    string decrypt(const string &plaintext, const string &key);

    map<string, string> collectMapString(const char *filePath);

    map<string, string> encryptMap(const map<string, string> &srcMap, const string &key);

    map<string, string> decryptMap(const map<string, string> &dstMap, const string &key);

    bool checkInWhiteList(const string &strName);

    void writeMapStringToLocalFile(map<string, string> strMap, const char *filePath);


public:
    string              &key;
    map<string, string> stringMap;
    map<string, string> eMap;
    map<string, string> deMap;
    vector<string>      whiteList;


};


#endif //CRYPTO_DEMO_PROGUARD_H

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

    string encrypt(const string &plaintext, const string &key);

    string decrypt(const string &plaintext, const string &key);

    map<string, string> collectMapString(const char *filePath);

    void writeMapStringToLocalFile(map<string, string> strMap, const char *filePath);


public:
    string              &key;
    map<string, string> stringMap;


};


#endif //CRYPTO_DEMO_PROGUARD_H

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

    Proguard();

    ~Proguard();

    void testCaseOne(const string &plaintext, const string &key, const string &iv);


    map<string, string> collectMapString(const char *filePath);

    map<string, string> encryptMap(const map<string, string> &srcMap, const string &m_key, const string &m_iv);

    map<string, string> decryptMap(const map<string, string> &dstMap, const string &m_key, const string &m_iv);

    bool checkInWhiteList(const string &strName);

    void writeMapStringToLocalFile(map<string, string> strMap, const char *filePath);


    std::string encryptAES_CBC(const std::string &plaintext, const std::string &key, const std::string &iv);

    std::string decryptAES_CBC(const std::string &ciphertext, const std::string &key, const std::string &iv);

    std::string hexEncode(const std::string &input);

    std::string hexDecode(const std::string &hexString);


public:
    string              mFlowers;
    map<string, string> stringMap;
    map<string, string> eMap;
    map<string, string> deMap;
    vector<string>      whiteList;


    const char *mKey = "4b57d4a9a3d7c2e135f6b4c7ec8b0d8c5a3e1f7d6c4a2b0d8f6b4c7e1f3d5b2c";

    const char *mIv = "a1b2c3d4e5f6g7h8";

};


#endif //CRYPTO_DEMO_PROGUARD_H

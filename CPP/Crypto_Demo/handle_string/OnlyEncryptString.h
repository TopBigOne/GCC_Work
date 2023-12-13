//
// Created by DEV on 11/30/23.
//

#ifndef CRYPTO_DEMO_ONLYENCRYPTSTRING_H
#define CRYPTO_DEMO_ONLYENCRYPTSTRING_H

#include <iostream>
using namespace std;

#define KEY_LENGTH 8

/**
 *
 */
class OnlyEncryptString {

public:
    OnlyEncryptString();
    string encryptString(const std::string &plaintext);

    string decryptString(const std::string &plaintext);

    string generateRandomKey(size_t keyLength);

    string hhheeexxxxDe(const char *hexString);

    string hhheeeE(const std::string &hexString);



private:
    string mFlowerK;
    string mFlowerI;


};


#endif //CRYPTO_DEMO_ONLYENCRYPTSTRING_H

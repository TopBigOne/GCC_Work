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
    string encryptString(const std::string &plaintext, const std::string &key);

    string decryptString(const std::string &plaintext, const std::string &key);
    string generateRandomKey(size_t keyLength);


};


#endif //CRYPTO_DEMO_ONLYENCRYPTSTRING_H

//
// Created by DEV on 10/23/23.
//

#ifndef CRYPTO_DEMO_TESTEVP_H
#define CRYPTO_DEMO_TESTEVP_H

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <iostream>
#include <cstdio>
#include <cstring>


using namespace std;


#define BUFFER_SIZE 4096


class TestEvp {

public:
    TestEvp();
    ~TestEvp();

    static void handleErrors() {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    void encryptFile(const unsigned char *key, const unsigned char *iv);

    void decryptFile(const unsigned char *key, const unsigned char *iv);

    void generateRandomKey(unsigned char *key);

    void printRandomKey();

public:
    char *inputFile = nullptr;
    char *middleOutputFile = nullptr;
    char *outputFile = nullptr;
    unsigned char key[16];
    unsigned char iv[16];

};


#endif //CRYPTO_DEMO_TESTEVP_H

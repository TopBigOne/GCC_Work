//
// Created by DEV on 11/30/23.
//


#include "OnlyEncryptString.h"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <sstream>
#include <cstring>
#include <openssl/err.h>



OnlyEncryptString::OnlyEncryptString() {

    mFlowerK.append("4b57d4a9a3d7c2e135f6b4c7ec8b0d8c5a3e1f7d6c4a2b0d8f6b4c7e1f3d5b2c");
    mFlowerI.append("a1b2c3d4e5f6g7h8");
}


string OnlyEncryptString::generateRandomKey(size_t keyLength) {
    unsigned char buffer[keyLength];
    RAND_bytes(buffer, keyLength);
    return std::string(reinterpret_cast<char*>(buffer), keyLength);


}



string OnlyEncryptString::encryptString(const string &plaintext) {

    //  LOGD(__func__)
    //  LOGI("  plaintext : %s", plaintext.c_str())
    const char *key = mFlowerK.c_str();
    const char *iv  = mFlowerI.c_str();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key),
                       reinterpret_cast<const unsigned char *>(iv));

    int         plaintextLength = plaintext.length();
    std::string ciphertext;
    int         aes_block_size  = 16;
    ciphertext.resize(plaintextLength + aes_block_size);

    int len = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]), &len,
                      reinterpret_cast<const unsigned char *>(plaintext.c_str()), plaintextLength);

    int finalLen = 0;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]) + len, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(len + finalLen);

    return hhheeeE(ciphertext);
}



string OnlyEncryptString::decryptString (const string &ciphertext) {
    const char *key = mFlowerK.c_str();
    const char *iv  = mFlowerI.c_str();

    std::string hexDecodeString = hhheeexxxxDe(ciphertext.c_str());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key),
                       reinterpret_cast<const unsigned char *>(iv));

    int         ciphertextLength = static_cast<int>(hexDecodeString.length());
    std::string plaintext;
    plaintext.resize(ciphertextLength);

    int len = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]), &len,
                      reinterpret_cast<const unsigned char *>(hexDecodeString.c_str()), ciphertextLength);

    int finalLen = 0;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]) + len, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(len + finalLen);

    // todo 注意指针释放
    char *vChar = new char[plaintext.length() + 1];
    strcpy(vChar, plaintext.c_str());
    delete[] vChar;
    // LOGI("  de plaintext : %s", plaintext.c_str())
    return plaintext;

}

string OnlyEncryptString::hhheeexxxxDe(const char *hexString) {
    string tempStr(hexString);
    std::string decodedString;
    for (size_t i = 0; i < tempStr.length(); i += 2) {
        std::string   byteString = tempStr.substr(i, 2);
        unsigned char byte       = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        decodedString += byte;
    }

    return decodedString;


}

string OnlyEncryptString::hhheeeE(const string &input) {
    std::string       hexString;
    static const char hexChars[] = "0123456789ABCDEF";

    for (unsigned char c: input) {
        hexString += hexChars[c >> 4];
        hexString += hexChars[c & 0xf];
    }

    return hexString;
}



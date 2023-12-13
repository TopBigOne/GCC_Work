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


string OnlyEncryptString::generateRandomKey(size_t keyLength) {
    unsigned char buffer[keyLength];
    RAND_bytes(buffer, keyLength);
    return std::string(reinterpret_cast<char*>(buffer), keyLength);


}

std::string base64_decode(const std::string &input) {
    BIO *bio, *b64;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
    bio = BIO_push(b64, bio);

    char *buffer = new char[input.length()];
    BIO_read(bio, buffer, static_cast<int>(input.length()));
    BIO_free_all(bio);

    std::string result(buffer, input.length());
    delete[] buffer;

    return result;
}

std::string base64_encode(const unsigned char *input, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, input, static_cast<int>(length));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}



string OnlyEncryptString::encryptString(const string &message, const string &secret_key) {
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    const int keyLength = EVP_CIPHER_key_length(cipher);
    const int ivLength = EVP_CIPHER_iv_length(cipher);

    // Ensure secret_key is the correct length
    unsigned char key[keyLength];
    memset(key, 0, keyLength);
    memcpy(key, secret_key.c_str(), std::min<int>(secret_key.length(), keyLength));

    // Generate a random IV
    unsigned char iv[ivLength];
    RAND_bytes(iv, ivLength);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        // Handle error
        std::cerr << "EVP_EncryptInit_ex failed." << std::endl;
        return "";
    }

    int messageLength = static_cast<int>(message.length());
    int encryptedLength = messageLength + EVP_CIPHER_block_size(cipher);
    unsigned char *encryptedText = new unsigned char[encryptedLength];

    if (EVP_EncryptUpdate(ctx, encryptedText, &encryptedLength, reinterpret_cast<const unsigned char *>(message.c_str()), messageLength) != 1) {
        // Handle error
        std::cerr << "EVP_EncryptUpdate failed." << std::endl;
        delete[] encryptedText;
        return "";
    }

    int finalLength;
    if (EVP_EncryptFinal_ex(ctx, encryptedText + encryptedLength, &finalLength) != 1) {
        // Handle error
        std::cerr << "EVP_EncryptFinal_ex failed." << std::endl;
        delete[] encryptedText;
        return "";
    }

    EVP_CIPHER_CTX_free(ctx);

    encryptedLength += finalLength;

    // Concatenate IV and encrypted text, then base64 encode the result
    std::string result = base64_encode(iv, ivLength) + base64_encode(encryptedText, encryptedLength);

    delete[] encryptedText;

    return result;
}



string OnlyEncryptString::decryptString (const string &ciphertext, const string &secret_key) {
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    const int keyLength = EVP_CIPHER_key_length(cipher);
    const int ivLength = EVP_CIPHER_iv_length(cipher);

    // Ensure secret_key is the correct length
    unsigned char key[keyLength];
    memset(key, 0, keyLength);
    memcpy(key, secret_key.c_str(), std::min<int>(secret_key.length(), keyLength));

    // Base64 decode the ciphertext
    std::string decodedCiphertext = base64_decode(ciphertext);

    // Extract IV from the decoded ciphertext
    unsigned char iv[ivLength];
    memcpy(iv, decodedCiphertext.c_str(), ivLength);

    // Initialize decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
        // Handle error
        std::cerr << "EVP_DecryptInit_ex failed." << std::endl;
        return "";
    }

    int decryptedLength;
    int ciphertextLength = static_cast<int>(decodedCiphertext.length()) - ivLength;
    unsigned char *decryptedText = new unsigned char[ciphertextLength];



    if (EVP_DecryptUpdate(ctx, decryptedText, &decryptedLength, reinterpret_cast<const unsigned char *>(decodedCiphertext.c_str() + ivLength), ciphertextLength) != 1) {
        // Handle error
        std::cerr << "EVP_DecryptUpdate failed." << std::endl;
        delete[] decryptedText;
        return "";
    }

    int finalLength;
    if (EVP_DecryptFinal_ex(ctx, decryptedText + decryptedLength, &finalLength) <= 0) {
        // Check for decryption failure or need more data
        int err = ERR_get_error();
        if (err != 0) {
            // Print OpenSSL error
            char errBuff[256];
            ERR_error_string_n(err, errBuff, sizeof(errBuff));
            std::cerr << "EVP_DecryptFinal_ex failed: " << errBuff << std::endl;
        }

        delete[] decryptedText;
        return "";
    }

    EVP_CIPHER_CTX_free(ctx);

    decryptedLength += finalLength;

    // Convert decrypted text to string
    std::string result(reinterpret_cast<char *>(decryptedText), decryptedLength);

    delete[] decryptedText;

    return result;



}

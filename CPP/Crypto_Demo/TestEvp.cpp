//
// Created by DEV on 10/23/23.
//

#include "TestEvp.h"
#include "zlib.h"



void TestEvp::decryptFile(const unsigned char *key, const unsigned char *iv) {

    printf("decryptFile.\n");
    FILE *ciphertextFile = fopen(middleOutputFile, "rb");
    if (!ciphertextFile) {
        printf("    Failed to open ciphertext file.\n");
        return;
    }

    printf("    outputFile :%s\n", outputFile);
    FILE *plaintextFile = fopen(outputFile, "wb");
    if (!plaintextFile) {
        printf("    Failed to create plaintext file.\n");
        fclose(ciphertextFile);
        return;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printf("    Failed to create cipher context.\n");
        fclose(ciphertextFile);
        fclose(plaintextFile);
        return;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        printf("Failed to initialize decryption.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(ciphertextFile);
        fclose(plaintextFile);
        return;
    }

    unsigned char ciphertextBuffer[BUFFER_SIZE];
    unsigned char plaintextBuffer[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int bytesRead = 0;
    int plaintextLength = 0;

    while ((bytesRead = fread(ciphertextBuffer, 1, BUFFER_SIZE, ciphertextFile)) > 0) {
        if (EVP_DecryptUpdate(ctx, plaintextBuffer, &plaintextLength, ciphertextBuffer, bytesRead) != 1) {
            printf("Decryption failed.\n");
            EVP_CIPHER_CTX_free(ctx);
            fclose(ciphertextFile);
            fclose(plaintextFile);
            return;
        }

        fwrite(plaintextBuffer, 1, plaintextLength, plaintextFile);
    }
    if (EVP_DecryptFinal_ex(ctx, plaintextBuffer, &plaintextLength) != 1) {
        printf("Decryption finalization failed.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(ciphertextFile);
        fclose(plaintextFile);
        return;
    }

    fwrite(plaintextBuffer, 1, plaintextLength, plaintextFile);

    EVP_CIPHER_CTX_free(ctx);
    fclose(ciphertextFile);
    fclose(plaintextFile);

    printf("    Decryption completed successfully.\n");


}

void TestEvp::encryptFile(const unsigned char *key, const unsigned char *iv) {
    puts("encryptFile");
    printf("    inputFile       : %s\n", inputFile);

    FILE *plaintextFile = fopen(inputFile, "rb");
    if (!plaintextFile) {
        printf("Failed to open plaintext file.\n");
        return;
    }
    printf("    middleOutputFile : %s\n", middleOutputFile);
    FILE *ciphertextFile = fopen(middleOutputFile, "wb");
    if (ciphertextFile == nullptr) {
        fprintf(stderr, "Failed to create ciphertext file.\n");
        fclose(plaintextFile);
        return;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printf("Failed to create cipher context.\n");
        fclose(plaintextFile);
        fclose(ciphertextFile);
        return;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        printf("Failed to initialize encryption.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(plaintextFile);
        fclose(ciphertextFile);
        return;
    }

    unsigned char plaintextBuffer[BUFFER_SIZE];
    unsigned char ciphertextBuffer[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int bytesRead = 0;
    int ciphertextLength = 0;

    while ((bytesRead = fread(plaintextBuffer, 1, BUFFER_SIZE, plaintextFile)) > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertextBuffer, &ciphertextLength, plaintextBuffer, bytesRead) != 1) {
            printf("Encryption failed.\n");
            EVP_CIPHER_CTX_free(ctx);
            fclose(plaintextFile);
            fclose(ciphertextFile);
            return;
        }

        fwrite(ciphertextBuffer, 1, ciphertextLength, ciphertextFile);
    }

    if (EVP_EncryptFinal_ex(ctx, ciphertextBuffer, &ciphertextLength) != 1) {
        printf("    Encryption finalization failed.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(plaintextFile);
        fclose(ciphertextFile);
        return;
    }

    fwrite(ciphertextBuffer, 1, ciphertextLength, ciphertextFile);

    EVP_CIPHER_CTX_free(ctx);
    fclose(plaintextFile);
    fclose(ciphertextFile);

    printf("    Encryption completed successfully.\n");


}

void TestEvp::generateRandomKey(unsigned char *key) {
    RAND_bytes(key, 16);
}

void TestEvp::printRandomKey() {
    puts("printRandomKey : ");
    for (unsigned char i: key) {
        printf("%02X ", i);
    }
    printf("\n");
    puts("-------------------------");
    for (unsigned char i: iv) {
        printf("%02X ", i);
    }
    printf("\n");

}

TestEvp::TestEvp() {
    puts("TestEvp Init");


    const char *PROJECT_ROOT = "/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/";


    // case 1:
    string tempOne("pratice_openssl/ttt.zip");
    tempOne.insert(0, PROJECT_ROOT);
    inputFile = new char[tempOne.length() + 1];
    strcpy(inputFile, tempOne.c_str());

    // case 2:
    string tempTwo("/pratice_openssl/for_google_app");
    tempTwo.insert(0, PROJECT_ROOT);
    middleOutputFile = new char[tempTwo.length() + 1];
    strcpy(middleOutputFile, tempTwo.c_str());

    // case 3:
    string tempThree("/pratice_openssl/ttt3");
    tempThree.insert(0, PROJECT_ROOT);
    outputFile = new char[tempThree.length() + 1];
    strcpy(outputFile, tempThree.c_str());
    // print:
    printf("    inputFile        : %s\n", inputFile);
    printf("    middleOutputFile : %s\n", middleOutputFile);
    printf("    outputFile       : %s\n", outputFile);


    generateRandomKey(key);
    generateRandomKey(iv);
    printRandomKey();


}

TestEvp::~TestEvp() {
    free(inputFile);
    free(middleOutputFile);
    free(outputFile);
}



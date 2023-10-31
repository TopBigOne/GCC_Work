#include <iostream>
#include <fstream>
#include <zlib.h>
#include "openssl/aes.h"
#include <sys/stat.h>
#include "TestEvp.h"

#include "FlowManager.h"
#include "TestZlib.h"
#include "handle_string/Proguard.h"


using namespace std;

// 加密密钥，需要保密
const static unsigned char key[] = "0123456789abcdef";

string      PROJECT_ROOT("/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/");
std::string inputFilePath    = PROJECT_ROOT.append("pratice_openssl/ttt.zip");
std::string encryptedFile    = PROJECT_ROOT.append("/pratice_openssl/for_google");
std::string decryptedFile    = PROJECT_ROOT.append("pratice_openssl/ttt2.zip");
std::string decompressedFile = PROJECT_ROOT.append("pratice_openssl/ttt3");

void testEvpCase();

void testNormal();

void testZlib();

// FlowManager
void testFlowManager();


/**
 * 加密函数
 * @param inputFile
 * @param outputFile
 */
void encryptFile(const std::string &inputFile, const std::string &outputFile) {
    puts("Step 1: 加密");
    std::ifstream ifs(inputFile, std::ios::binary);
    std::ofstream ofs(outputFile, std::ios::binary);

    // 设置加密算法上下文
    AES_KEY aesKey;
    AES_set_encrypt_key(key, 128, &aesKey);

    unsigned char inBlock[AES_BLOCK_SIZE];
    unsigned char outBlock[AES_BLOCK_SIZE];

    // 逐块加密文件数据
    while (ifs.read(reinterpret_cast<char *>(inBlock), AES_BLOCK_SIZE)) {
        AES_encrypt(inBlock, outBlock, &aesKey);
        ofs.write(reinterpret_cast<const char *>(outBlock), AES_BLOCK_SIZE);
    }

    // 处理最后不足一块的数据
    if (ifs.gcount() > 0) {
        std::fill(std::begin(inBlock) + ifs.gcount(), std::end(inBlock), 0);
        AES_encrypt(inBlock, outBlock, &aesKey);
        ofs.write(reinterpret_cast<const char *>(outBlock), AES_BLOCK_SIZE);
    }

    ifs.close();
    ofs.close();

    std::cout << " File encrypted successfully." << std::endl;
}

/**
 * 解密函数
 * @param inputFile
 * @param outputFile
 */
void decryptFile(const std::string &inputFile, const std::string &outputFile) {
    puts("Step 2: 解密");
    std::ifstream ifs(inputFile, std::ios::binary);
    std::ofstream ofs(outputFile, std::ios::binary);

    // 设置解密算法上下文
    AES_KEY aesKey;
    AES_set_decrypt_key(key, 128, &aesKey);

    unsigned char inBlock[AES_BLOCK_SIZE];
    unsigned char outBlock[AES_BLOCK_SIZE];

    // 逐块解密文件数据
    while (ifs.read(reinterpret_cast<char *>(inBlock), AES_BLOCK_SIZE)) {
        AES_decrypt(inBlock, outBlock, &aesKey);
        ofs.write(reinterpret_cast<const char *>(outBlock), AES_BLOCK_SIZE);
    }

    // 处理最后不足一块的数据
    if (ifs.gcount() > 0) {
        ofs.write(reinterpret_cast<const char *>(outBlock), ifs.gcount());
    }

    ifs.close();
    ofs.close();

    std::cout << "  File decrypted successfully." << std::endl;
}


bool decompressZipFile(const std::string &zipFile, const std::string &destDir) {

    puts("Step 3: 解压");
    int result = mkdir(destDir.c_str(), 0777);
    if (result != 0) {
        std::cout << "Failed to create directory: " << destDir << std::endl;

    }

    gzFile zip = gzopen(zipFile.c_str(), "rb");
    if (!zip) {
        std::cout << "Failed to open ZIP file" << std::endl;
        return false;
    }

    char buffer[4096];

    while (true) {
        int bytesRead = gzread(zip, buffer, sizeof(buffer));
        if (bytesRead < 0) {
            std::cout << "Failed to read ZIP file" << std::endl;
            gzclose(zip);
            return false;
        }
        if (bytesRead == 0)
            break;

        std::string fileName = zipFile.substr(zipFile.find_last_of("/") + 1);

        std::string filePath = destDir + "/" + fileName;

        std::ofstream outputFile(filePath, std::ios::binary | std::ios::app);
        if (!outputFile) {
            std::cout << "Failed to create file: " << filePath << std::endl;
            gzclose(zip);
            return false;
        }

        outputFile.write(buffer, bytesRead);
        outputFile.close();
    }

    gzclose(zip);

    std::cout << "Decompression complete" << std::endl;
    return true;

}


void testNormal() {
    encryptFile(inputFilePath, encryptedFile);

    decryptFile(encryptedFile, decryptedFile);


    decompressZipFile(decryptedFile, decompressedFile);

}

/**
 * 测试加密解密
 */
void testEvpCase() {
    auto *testEvp = new TestEvp;
    //testEvp->encryptFile(testEvp->key, testEvp->iv);
    // testEvp->decryptFile(testEvp->key, testEvp->iv);
}


int test_fopen() {
    // input.txt
    FILE *file = fopen("/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/example.bin", "wb");
    if (file == NULL) {
        printf("Failed to create binary file.\n");
        return 1;
    }

    // 写入二进制数据
    unsigned char data[] = {0x12, 0x34, 0x56, 0x78};
    fwrite(data, sizeof(unsigned char), sizeof(data), file);

    fclose(file);

    printf("Binary file created successfully.\n");

    return 0;
}


void testZlib() {
    fputs("testZlib", stdout);
    TestZlib testZlib;
    char     *inFilename     = "/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/pratice_openssl/test_classes.dex";
    char     *middleFilename = "/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/pratice_openssl/ttt_middle";
    char     *outfilename    = "/Users/dev/Documents/GCC_Work/CPP/Crypto_Demo/pratice_openssl/Zlib_result.dex";

    printf("file size : %ld", testZlib.file_size(inFilename));

    testZlib.compress_one_file(inFilename, middleFilename);
    testZlib.decompress_one_file(middleFilename, outfilename);
}


void testFlowManager() {
    FlowManager flowManager;


    // 压缩
    flowManager.CompressDex();
    // 加密
    flowManager.handleEncryptZip();

    // 将加密文件copy到android的asset中
    flowManager.moveEncryptFileToAssert();

    // 解密
    flowManager.handleDecipher();
    // 解压
    flowManager.DecompressDex();
}

void testProguardString() {
    puts("testProguardString");
    constexpr const char *z_a = "Zr";
    constexpr const char *z_b = "H";
    constexpr const char *z_c = "ar";
    constexpr const char *z_d = "ns";
    constexpr const char *z_e = "G";

    constexpr const char *a1 = "J";
    constexpr const char *b2 = "io";
    constexpr const char *c3 = "N";
    constexpr const char *e5 = "Gy";


    string bullet;
    bullet.append(z_a);
    bullet.append(z_b);
    bullet.append(z_c);
    bullet.append(z_d);
    bullet.append(z_e);
    bullet.append(z_e);
    bullet.append(a1);
    bullet.append(b2);
    bullet.append(c3);
    bullet.append(e5);


    string key_one("jing zhang");


    Proguard proguard;
    string   one("com/jar/MainApplication");

    proguard.testCaseOne(one, bullet);


    string documents_root(
            "/Users/dev/Documents/Andorid_Work/Work_1/TestAssetManager/app/src/main/cpp/lib_TransformStr/");
    string source("SSConstant.h");
    source.insert(0, documents_root);

    string target("ZZConstant.h");
    target.insert(0, documents_root);


    printf("    bullet: %s\n", bullet.c_str());
    // step 1:
    map<string, string> collectMap = proguard.collectMapString(source.c_str());

    // step 2:
    map<string, string> encryptMapResult = proguard.encryptMap(collectMap, bullet);

    // step 3:
    proguard.writeMapStringToLocalFile(encryptMapResult, target.c_str());
}


void testProguardString2() {
    puts("testProguardString2");
    Proguard proguard;

    string raw("com/jar/MainApplication");

    string encryptResult = proguard.encryptAES_CBC(raw, proguard.mKey, proguard.mIv);
    cout << "    encryptResult : " << encryptResult << endl;

    string hexDecode = proguard.hexDecode(encryptResult);

    string decryptResult = proguard.decryptAES_CBC(hexDecode, proguard.mKey, proguard.mIv);
    cout << "    decryptResult : " << decryptResult << endl;

    // BD5811E33141FE8F7B225589007C9BE1B45A3EC609CB34434D97A8D8D907A79C
    // BD5811E33141FE8F7B225589007C9BE1B45A3EC609CB34434D97A8D8D907A79C
}

int main() {
    // testNormal();
    // testEvpCase();
    //  test_fopen();
    //  testZlib();
    // testFlowManager();
    // testProguardString();
    // 测试第二种
    testProguardString2();


    return 0;
}

//
// Created by DEV on 10/27/23.
//

#include <openssl/aes.h>
#include "Proguard.h"
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "openssl/des.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"


Proguard::Proguard() {
    whiteList.emplace_back("z_a");
    whiteList.emplace_back("z_b");
    whiteList.emplace_back("z_c");
    whiteList.emplace_back("z_d");
    whiteList.emplace_back("z_e");

    whiteList.emplace_back("a1");
    whiteList.emplace_back("b2");
    whiteList.emplace_back("c3");
    whiteList.emplace_back("e5");

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

    mFlowers.append(bullet);

    unsigned char key_data[] = {
            0x4b, 0x57, 0xd4, 0xa9, 0xa3, 0xd7, 0xc2, 0xe1,
            0x35, 0xf6, 0xb4, 0xc7, 0xec, 0x8b, 0x0d, 0x8c,
            0x5a, 0x3e, 0x1f, 0x7d, 0x6c, 0x4a, 0x2b, 0x0d,
            0x8f, 0x6b, 0x4c, 0x7e, 0x1f, 0x3d, 0x5b, 0x2c
    };

    std::copy(std::begin(key_data), std::end(key_data), mKey);


}


Proguard::~Proguard() {

}


/**
 * 收集头文件里的字符串
 * @param filePath
 * @return
 */
map<string, string> Proguard::collectMapString(const char *filePath) {
    cout << "collectMapString" << endl;

    std::ifstream file(filePath);  // 替换为你的头文件路径

    if (!file.is_open()) {
        cerr << "   Failed to open the file." << endl;
        return stringMap;

    }

    std::string line;


    // std::regex pattern("constexpr\\s+const\\s+char\\s*\\*(\\w+)\\s*=\\s*\"([^\"]*)\";");
    std::regex pattern(R"lit(constexpr\s+const\s+char\s*\*(\w+)\s*=\s*"([^"]*)";)lit");

    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_match(line, match, pattern)) {
            std::string name    = match[1].str();
            std::string content = match[2].str();

            stringMap[name] = content;
            cout << "   Name: " << name << ", Content: " << content << endl;
        }
    }
    file.close();


    return stringMap;
}

void Proguard::writeMapStringToLocalFile(map<string, string> strMap, const char *filePath) {
    cout << "writeMapStringToLocalFile" << endl;
    std::ofstream outfile(filePath);

    if (!outfile.is_open()) {
        std::cerr << "  Failed to open ZZConstant.h for writing." << std::endl;
        return;
    }

    outfile << "//" << std::endl;
    outfile << "// Created by DEV on 10/30/23." << std::endl;
    outfile << "//" << std::endl;
    outfile << "#ifndef TESTASSETMANAGER_ZZCONSTANT_H" << std::endl;
    outfile << "#define TESTASSETMANAGER_ZZCONSTANT_H" << std::endl;
    outfile << std::endl;
    outfile << std::endl;
    outfile << "#include \"StringCompileConfig.h\"" << std::endl;
    outfile << std::endl;
    outfile << std::endl;
    outfile << "#ifndef  USE_DEBUG_STRING_VERSION" << std::endl;

    outfile << std::endl;
    for (const auto &item: strMap) {
        std::string name    = item.first;
        std::string content = item.second;
        outfile << "constexpr const char *" << name << " = \"" << content << "\";" << endl;
    }
    outfile << std::endl;
    outfile << std::endl;
    outfile << std::endl;
    outfile << "#endif" << std::endl;
    outfile << std::endl;
    outfile << std::endl;
    outfile << "#endif" << std::endl;

    outfile.close();
    std::cout << "  Strings have been written to ZZConstant.h." << std::endl;

}

void Proguard::testCaseOne(const std::string &plaintext, const string &temp_key, const string &temp_iv) {
    cout << "testCaseOne" << endl;
    cout << "   key              : " << temp_key << endl;
    cout << "   before encrypt   : " << plaintext << endl;

    string resultOne = encryptAES_CBC(plaintext, temp_key, temp_iv);
    cout << "   after encrypt    : " << resultOne << endl;
    string resultTwo = decryptAES_CBC(resultOne, temp_key, temp_iv);
    cout << "   after decrypt    : " << resultTwo << endl;
}

bool Proguard::checkInWhiteList(const std::string &strName) {
    //  cout << "checkInWhiteList " << endl;
    auto it = std::find(whiteList.begin(), whiteList.end(), strName);
    return !(it == whiteList.end());
}


map<string, string> Proguard::encryptMap(const map<string, string> &srcMap, const string &m_key, const string &m_iv) {
    eMap.clear();
    for (const auto &item: srcMap) {
        string k = item.first;
        if (checkInWhiteList(k)) {
            // cout << "    " << k << " is in white list." << endl;
            eMap[k] = item.second;
            continue;
        }
        string v = encryptAES_CBC(item.second, m_key, m_iv);
        eMap[k] = v;

    }
    return eMap;
}

map<string, string> Proguard::decryptMap(const map<string, string> &dstMap, const string &m_key, const string &m_iv) {
    deMap.clear();
    for (const auto &item: dstMap) {
        string k = item.first;
        if (checkInWhiteList(k)) {
            continue;
        }
        string v = decryptAES_CBC(item.second, m_key, m_iv);
        deMap[k] = v;
    }
    return deMap;
}

std::string Proguard::encryptAES_CBC(const string &plaintext, const string &key, const string &iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()),
                       reinterpret_cast<const unsigned char *>(iv.c_str()));

    int         plaintextLength = plaintext.length();
    std::string ciphertext;
    ciphertext.resize(plaintextLength + AES_BLOCK_SIZE);

    int len = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]), &len,
                      reinterpret_cast<const unsigned char *>(plaintext.c_str()), plaintextLength);

    int finalLen = 0;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]) + len, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(len + finalLen);

    return hexEncode(ciphertext);

}

std::string Proguard::decryptAES_CBC(const string &ciphertext, const string &key, const string &iv) {


    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()),
                       reinterpret_cast<const unsigned char *>(iv.c_str()));

    int         ciphertextLength = ciphertext.length();
    std::string plaintext;
    plaintext.resize(ciphertextLength);

    int len = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]), &len,
                      reinterpret_cast<const unsigned char *>(ciphertext.c_str()), ciphertextLength);

    int finalLen = 0;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]) + len, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(len + finalLen);

    return plaintext;
}

std::string Proguard::hexEncode(const string &input) {
    std::string       hexString;
    static const char hexChars[] = "0123456789ABCDEF";

    for (unsigned char c: input) {
        hexString += hexChars[c >> 4];
        hexString += hexChars[c & 0xf];
    }

    return hexString;
}

std::string Proguard::hexDecode(const string &hexString) {
    std::string decodedString;

    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::string   byteString = hexString.substr(i, 2);
        unsigned char byte       = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        decodedString += byte;
    }

    return decodedString;

}



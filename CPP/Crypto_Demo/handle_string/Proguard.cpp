//
// Created by DEV on 10/27/23.
//

#include "Proguard.h"


Proguard::Proguard(string &key) : key(key) {


    whiteList.emplace_back("z_a");
    whiteList.emplace_back("z_b");
    whiteList.emplace_back("z_c");
    whiteList.emplace_back("z_d");
    whiteList.emplace_back("z_e");

    whiteList.emplace_back("a1");
    whiteList.emplace_back("b2");
    whiteList.emplace_back("c3");
    whiteList.emplace_back("e5");

}

Proguard::~Proguard() {

}


string Proguard::encrypt(const string &plaintext, const string &key) {
    // 初始化加密上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), nullptr);

    // 设置输入数据长度和输出缓冲区
    int           inputLength     = plaintext.length();
    int           maxOutputLength = inputLength + EVP_CIPHER_CTX_block_size(ctx);
    unsigned char *ciphertext     = new unsigned char[maxOutputLength];
    int           outputLength    = 0;

    // 执行加密操作
    EVP_EncryptUpdate(ctx, ciphertext, &outputLength, reinterpret_cast<const unsigned char *>(plaintext.c_str()),
                      inputLength);

    // 结束加密操作
    int finalOutputLength = 0;
    EVP_EncryptFinal_ex(ctx, ciphertext + outputLength, &finalOutputLength);
    outputLength += finalOutputLength;

    // 清理加密上下文
    EVP_CIPHER_CTX_free(ctx);

    // 将加密结果转换为 Base64 编码
    BIO *bio = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, ciphertext, outputLength);
    BIO_flush(bio);

    // 读取 Base64 编码后的结果
    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encryptedText(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    delete[] ciphertext;

    return encryptedText;

}

string Proguard::decrypt(const string &encryptedText, const string &key) {
    // 将 Base64 编码后的加密结果解码
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new_mem_buf(encryptedText.c_str(), encryptedText.length());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    // 计算解码后的长度
    int  decodedLength = BIO_pending(bio);
    char *decodedText  = new char[decodedLength];

    // 解码操作
    BIO_read(bio, decodedText, encryptedText.length());

    // 清理 BIO
    BIO_free_all(bio);

    // 初始化解密上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), nullptr);

    // 设置输入数据长度和输出缓冲区
    int           inputLength     = decodedLength;
    int           maxOutputLength = inputLength;
    unsigned char *plaintext      = new unsigned char[maxOutputLength];
    int           outputLength    = 0;

    // 执行解密操作
    EVP_DecryptUpdate(ctx, plaintext, &outputLength, reinterpret_cast<const unsigned char *>(decodedText), inputLength);

    // 结束解密操作
    int finalOutputLength = 0;
    EVP_DecryptFinal_ex(ctx, plaintext + outputLength, &finalOutputLength);
    outputLength += finalOutputLength;

    // 移除填充数据
    int paddingSize = plaintext[outputLength - 1];
    outputLength -= paddingSize;

    // 清理解密上下文
    EVP_CIPHER_CTX_free(ctx);

    std::string decryptedText(reinterpret_cast<char *>(plaintext), outputLength);

    delete[] plaintext;
    delete[] decodedText;

    return decryptedText;


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
    for (const auto &item: strMap) {
        std::string name    = item.first;
        std::string content = item.second;
        outfile << "constexpr const char *" << name << " = \"" << content << "\";" << endl;
    }
    outfile << std::endl;
    outfile << std::endl;
    outfile << std::endl;
    outfile << "#endif" << std::endl;

    outfile.close();
    std::cout << "  Strings have been written to ZZConstant.h." << std::endl;

}

void Proguard::testCaseOne(const std::string &plaintext, const std::string &temp_key) {
    cout << "testCaseOne" << endl;
    cout << "   key              : " << temp_key << endl;
    cout << "   before encrypt   : " << plaintext << endl;

    string resultOne = encrypt(plaintext, temp_key);
    cout << "   after encrypt    : " << resultOne << endl;
    string resultTwo = decrypt(resultOne, temp_key);
    cout << "   after decrypt    : " << resultTwo << endl;
}

bool Proguard::checkInWhiteList(const std::string &strName) {
    cout << "checkInWhiteList " << endl;
    auto it = std::find(whiteList.begin(), whiteList.end(), strName);
    return !(it == whiteList.end());
}


map<string, string> Proguard::encryptMap(const map<string, string> &srcMap, const string &temp_key) {
    eMap.clear();
    for (const auto &item: srcMap) {
        string k = item.first;
        if (checkInWhiteList(k)) {
            cout << "    " << k << " is in white list." << endl;
            eMap[k] = item.second;
            continue;
        }
        string v = encrypt(item.second, temp_key);
        eMap[k] = v;

    }
    return eMap;
}

map<string, string> Proguard::decryptMap(const map<string, string> &dstMap, const string &temp_key) {
    deMap.clear();
    for (const auto &item: dstMap) {
        string k = item.first;
        if (checkInWhiteList(k)) {
            continue;
        }
        string v = decrypt(item.second, temp_key);
        deMap[k] = v;
    }
    return deMap;
}


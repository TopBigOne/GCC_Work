#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>


void testOne();

std::string calculateMD5(std::string basicString);

int main() {
    testOne();
    return 0;
}

void testOne() {
    std::string input   = "Hello, World!";
    std::string md5Hash = calculateMD5(input);
    std::cout << "MD5 Hash: " << md5Hash << std::endl;
}

std::string calculateMD5(std::string str) {

    unsigned char md[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char *>(str.c_str()), str.length(), md);

    std::stringstream ss;
    for (int          i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md[i]);
    }

    return ss.str();

}

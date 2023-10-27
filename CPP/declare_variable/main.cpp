#include <iostream>
#include <string>

using namespace std;

void modifyStringReference(string &strRef) {
    strRef = "Modified via reference";
}

/**
 * 而 modifyString 函数接收一个 string 值，它在函数内部创建了一个新的字符串副本，
 * 并对副本进行修改。因此，对副本的修改不会影响到原始字符串。
 * @param str
 */
void modifyString(string str) {
    str = "Modified";
}

int main() {
    string original = "Original";
    string &ref     = original;


    modifyStringReference(ref);
    std::cout << "Reference : " << original << std::endl;


    std::string value = original;

    modifyString(value);

    std::cout << "Value    : " << value << std::endl;
    std::cout << "original : " << original << std::endl;


    return 0;
}

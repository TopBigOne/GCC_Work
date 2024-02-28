#include <iostream>
#include <sstream>

void test_1();

void test_2();

void test_1() {
    puts("将不同类型的数据转换为字符串 : ");
    int               num  = 42;
    float             pi   = 3.14;
    std::string       name = "xiao ya";
    // 将不同类型的数据转为字符串
    std::stringstream str_stream;
    str_stream << " Number : " << num << ", Pi: " << pi << " , Name :" << name;
    std::string result = str_stream.str();
    std::cout << result << std::endl;

}


void test_2() {
    puts("将字符串解析为不同的数据类型: ");
    std::string number_str = "123";
    int         parsedNumber;

    std::stringstream result(number_str);
    result >> parsedNumber;
    std::cout << "Parsed number : " << parsedNumber << std::endl;


}


/**
 * sstream : 提供了一种方便的方式来处理字符串和其他数据类型之间的转换。
 * @return
 */
int main() {
    test_1();
    test_2();

    return 0;
}

#include <iostream>

using namespace std;
union Data {
    bool val1;
    char val2;
    int  val3;
    long val4;
};

int main() {
    Data data{};
    data.val3 = 1300;
    cout << "背景知识点：" << endl;
    cout << "char    sizeof : " << sizeof(char) << endl;
    cout << "short   sizeof : " << sizeof(short) << endl;
    cout << "int     sizeof : " << sizeof(int) << endl;
    cout << "long    sizeof : " << sizeof(long) << endl;
    cout << "float  sizeof  : " << sizeof(float) << endl;
    cout << "double  sizeof : " << sizeof(double) << endl;
    cout << "-----------------------------------------------" << endl;

    cout << "size of(data) " << sizeof(data) << endl;
    cout << "val1 : " << data.val1 << endl;
    cout << "val2 : " << data.val2 << endl;


    return 0;
}

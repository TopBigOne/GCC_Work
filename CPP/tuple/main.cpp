#include <iostream>

using namespace std;
string name;
tuple<string &, int> tpRef(name, 30);

struct person {
    int age;
    char *name;
    char *ages;
    // 可以用tuple来表示这样的一个结构类型，作用是一样的
    //  std::tuple<const char *, const char *, int>
};


void test_one();

void test_two();

void test_three();

void test_four();

int main() {
    test_one();
    test_two();
    test_three();
    test_four();
    return 0;
}


void test_one() {
    puts("case 1: tuple的元素类型为引用");
    // 对tpRef第一个元素赋值，同时name也被赋值 - 引用
    get<0>(tpRef) = "jar";

    cout << "   name : " << name << '\n';
    cout << "   age  : " << get<1>(tpRef) << '\n';
}


void test_two() {
    puts("case 2: 如何获取tuple元素个数");
    tuple<int, char, double> my_tuple(10, 'j', 3.14);
    int element_size = tuple_size<decltype(my_tuple)>::value;
    cout << "   tuple 元素的个数：" << element_size << endl;
}

void test_three() {
    puts("case 3: 如何获取元素的值");
    tuple<int, char, double> my_tuple(10, 'j', 3.14);
    cout << "     the element is ";
    cout << get<0>(my_tuple) << " ";
    cout << get<1>(my_tuple) << " ";
    cout << get<2>(my_tuple) << " ";
}

void test_four() {
    puts("case 4: 获取元素的类型");
    std::tuple<int, string> tp(28, "jar jar");
    tuple_element<0, decltype(tp)>::type ages_type;
    ages_type = get<0>(tp);
    cout << "     ages_type :" << ages_type << endl;
}

/**
 * 如同pair一样也是可以通过tie进行解包tuple的各个元素的值。
 * 如下tuple对象有4个元素，通过tie解包将会把这4个元素的值分别赋值给tie提供的4个变量中。
 */
void test_five() {
    puts("case 5: 利用tie进行解包元素的值");
    std::tuple<string, int, string, int> tp;
    tp = make_tuple("jar jar", 28, "hangzhou", 1995);
    string name, addr;
    int ages, born_year;

}



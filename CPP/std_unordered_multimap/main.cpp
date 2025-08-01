#include <iostream>
#include <unordered_map>
#include <string>
using namespace std;

void test_1() {
    const char *KEY_1 = "ClassA";
    const char *KEY_2 = "ClassB";
    unordered_multimap<string, int> class_student;
    class_student.insert({KEY_1, 1001});
    class_student.insert({KEY_1, 1002});
    class_student.insert({KEY_2, 4003});
    cout << "全部元素 : " << endl;
    for (auto student: class_student) {
        auto first = student.first;
        auto second = student.second;
        cout << "   first : " << first << " <-> second :" << second << endl;
    }
    cout << "查找所有ClassA 的学生 : " << endl;
    auto class_pair_a = class_student.equal_range("ClassA");
    for (auto it = class_pair_a.first; it != class_pair_a.second; ++it) {
        cout << "   second value :" << it->second << endl;
    }
    cout << "删除ClassB 的学生 : " << endl;
    class_student.erase(KEY_2);
    cout << "统计ClassA 的人数 : " << endl;
    auto class_a_count = class_student.count(KEY_1);
    cout << "   class_a_count :" << class_a_count << endl;
}

int main() {
    test_1();
    return 0;
}

#include <iostream>

#include <string>
#include "UseString.h"

using namespace std;

int main() {
    UseString useString;
    useString.raw_str = "qwertyuiop";
    string temp_str = useString.raw_str;
    cout << "temp_str   : " << temp_str << endl;
    // case 1:
    size_t size = useString.my_size(temp_str);
    cout << "size()     : " << size << endl;
    // case 2:
    size_t length = useString.my_length(temp_str);
    cout << "length()   : " << length << endl;
    // case 3:
    bool empty = useString.my_empty(temp_str);
    cout << "empty()    : " << empty << endl;
    // case 4:
    string my_substr = useString.my_substr(temp_str, 2, 7);
    cout << "substr()   : pos = 2,len = 7 " << my_substr << endl;

    // case 4:
    string my_append = useString.my_append(temp_str, "我是后来添加的");
    cout << "append()   : 尾部添加 " << my_append << endl;

    // case 5:
    string my_insert = useString.my_insert(temp_str, 2, "我是后来insert的");
    cout << "insert()   :  pos = 2  ：" << my_insert << endl;

    // case 6:
    string my_replace = useString.my_replace(temp_str, 5, 2, "俺是replace添加的");
    cout << "replace()  :  pos = 5 len = 2 " << my_replace << endl;

    // case 7:
    size_t my_find = useString.my_find(temp_str, "er");
    cout << "find()     :  " << my_find << endl;

    // case 7:
    const char *my_c_str = useString.my_c_str(temp_str);
    cout << "*c_str()       :  " << *my_c_str << endl;
    cout << " *(c_str()+1)  :  " << *(my_c_str + 1) << endl;
    cout << " *(c_str()+2)  :  " << *(my_c_str + 2) << endl;

    // case 8:
    const char my_at = useString.my_at(temp_str, 3);
    cout << "my_at()    :  index = 3 " << my_at << endl;



    return 0;
}

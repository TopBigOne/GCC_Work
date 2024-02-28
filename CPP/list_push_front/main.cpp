#include <iostream>
#include <list>

using namespace std;

int main() {
    list<int> mList;
    mList.push_front(23);
    mList.push_front(24);
    mList.push_front(25);

    for (const auto &item: mList) {
        cout << "item is :" << item << endl;
    }

    return 0;
}

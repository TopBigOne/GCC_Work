#include <iostream>

#include "thread"

using namespace std;

class Counter {
public:
    Counter(int id, int numIterations)
            : mId(id), mNumIterations(numIterations) {

    }

    //重载运算符operator()
    void operator()() const {
        for (int i = 0; i < mNumIterations; ++i) {
            cout << "Counter " << mId << " has value " << i << endl;
        }
    }

private:
    int mId;
    int mNumIterations;
};


int main() {
    // case 1: 通过构造函数创建Counter类的一个实例，将实例传递给thread类
    thread  t1{Counter{1, 4}};
    // 创建Counter类的一个实例c，将实例传递给thread类
    Counter c(2, 5);
    thread  t2(c);

    t1.join();
    t2.join();

    cout << "Main thread end." << endl;
    return 0;

    return 0;
}

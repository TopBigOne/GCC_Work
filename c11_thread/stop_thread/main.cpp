#include <iostream>

#include <thread>
#include <chrono>
#include <future>
#include <assert.h>

using namespace std;

void tempFunc(future<int> obj) {
    cout << "Thread Start..." << endl;
    // wait_for() 方法通常会在循环中使用，以便持续检查 future 对象的状态。这样可以在等待期间执行其他任务或定期检查结果是否可获取。
    while (obj.wait_for(chrono::microseconds(1)) == future_status::timeout) {
        cout << "Doing some work." << endl;
        // 3秒钟
        this_thread::sleep_for(chrono::microseconds(1000));
    }
    cout << "Thread End..." << endl;
}

void start_test() {
    promise<int> exitSignal;
    future<int> futureObj = exitSignal.get_future();

    thread th(tempFunc, std::move(futureObj));
    // wait 10s;
    this_thread::sleep_for(chrono::seconds(3));
    cout << "Ask thread to stop" << endl;
    // set promise value ： 2
    exitSignal.set_value(2);
    // 等待线程退出
    th.join();
    cout << "Exiting Main Function" << endl;
}


int main() {
    start_test();

    return 0;
}

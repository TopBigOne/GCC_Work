#include <iostream>
#include "ThreadExtension.h"

class TestThread : public ThreadExtension {
public:
    TestThread() {

    }

    virtual ~TestThread() {

    }

    void CustomRun() override {
        std::cout << "CustomRun" << std::endl;
    }
};


int main() {
    TestThread thread;
    std::cout << "Thread State:" << thread.GetThreadState() << std::endl;

    thread.StartThread();
    std::cout << "Thread State:" << thread.GetThreadState() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    thread.PauseThread();
    std::cout << "Thread State:" << thread.GetThreadState() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(11100));


    thread.ResumeThread();
    std::cout << "Thread State:" << thread.GetThreadState() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    thread.StopThread();
    std::cout << "Thread State:" << thread.GetThreadState() << std::endl;

    getchar();

    return 0;
}

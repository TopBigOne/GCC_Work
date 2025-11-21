#include <iostream>

#include <thread>
#include <future>

void printNote(const char *note);

void printNote(const char *note) {
    std::cout << note << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

void calc_square(std::promise<int> resultPromise, int x) {
    const int result = x * x;
    resultPromise.set_value(result);
}

void test1() {
    const auto note = "Demo Note：主线程启动一个工作线程，工作线程计算结果并通过 promise 发送给主线程，主线程通过 future 异步获取结果";

    printNote(note);
    // 主线程创建 promise 对象
    std::promise<int> my_promise;
    std::future<int> my_future = my_promise.get_future();
    int value = 10;
    // 启动线程计算
    std::thread worker_thread(calc_square, std::move(my_promise), value);
    // do other things...
    // 等待并发结果：future.get() 会阻塞，直到 promise 设置了值。
    const int result = my_future.get();
    std::cout << "square result : " << result << std::endl;
    worker_thread.join();
}

void cal_with_exception(std::promise<int> resultPromise) {
    try {
        throw std::runtime_error("something bad happened");
    } catch (const std::exception &e) {
        resultPromise.set_exception(std::current_exception());
    }
}


void test2() {
    const auto note = "Demo Note：异常传递";
    printNote(note);
    std::promise<int> my_promise;
    std::future<int> my_future = my_promise.get_future();
    // 开始线程计算
    std::thread work_thread(cal_with_exception, std::move(my_promise));

    try {
        // 会出现异常
        int result = my_future.get();
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    work_thread.join();
}


int main() {
    test1();
    test2();
    return 0;
}

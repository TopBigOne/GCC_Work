#include "demo_thread.h"

#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// 策略模式：将"异常处理方式"与"任务执行"解耦
struct IExceptionHandler {
    virtual void handle(std::exception_ptr ep, int thread_id) = 0;
    virtual ~IExceptionHandler() = default;
};

struct PrintHandler : IExceptionHandler {
    void handle(std::exception_ptr ep, int thread_id) override {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            std::cout << "  [线程 " << thread_id << "] 异常: " << e.what() << "\n";
        }
    }
};

static void worker(int id, std::exception_ptr& out_ep, std::mutex& mtx) {
    try {
        if (id % 2 == 0) {
            throw std::runtime_error("线程 " + std::to_string(id) + " 运行失败");
        }
        std::cout << "  [线程 " << id << "] 正常完成\n";
    } catch (...) {
        std::lock_guard<std::mutex> lk(mtx);
        out_ep = std::current_exception();
    }
}

void DemoThread::run() {
    std::cout << "\n[demo_thread] 跨线程异常传递\n";

    const int N = 4;
    std::vector<std::exception_ptr> exceptions(N);
    std::vector<std::thread> threads;
    std::mutex mtx;

    for (int i = 0; i < N; ++i) {
        threads.emplace_back(worker, i, std::ref(exceptions[i]), std::ref(mtx));
    }
    for (auto& t : threads) t.join();

    PrintHandler handler;
    for (int i = 0; i < N; ++i) {
        if (exceptions[i]) {
            handler.handle(exceptions[i], i);
        }
    }
}

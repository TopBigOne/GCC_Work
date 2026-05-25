#include "demo_aggregator.h"

#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// 命令模式：封装一个可能抛异常的任务单元
class SafeTask {
public:
    SafeTask(std::string name, std::function<void()> fn)
        : name_(std::move(name)), fn_(std::move(fn)) {}

    void execute() const { fn_(); }
    const std::string& name() const { return name_; }

private:
    std::string name_;
    std::function<void()> fn_;
};

// 聚合模式：收集所有任务执行中产生的异常，统一汇报
class ExceptionAggregator {
public:
    void submit(SafeTask task) {
        try {
            task.execute();
        } catch (...) {
            records_.push_back({task.name(), std::current_exception()});
        }
    }

    bool has_errors() const { return !records_.empty(); }

    void report() const {
        for (const auto& r : records_) {
            try {
                std::rethrow_exception(r.ep);
            } catch (const std::exception& e) {
                std::cerr << "  [" << r.task_name << "] " << e.what() << "\n";
            }
        }
    }

private:
    struct Record {
        std::string task_name;
        std::exception_ptr ep;
    };
    std::vector<Record> records_;
};

void DemoAggregator::run() {
    std::cout << "\n[demo_aggregator] 异常聚合器\n";

    ExceptionAggregator agg;
    agg.submit(SafeTask("任务A", [] { std::cout << "  任务A: 成功\n"; }));
    agg.submit(SafeTask("任务B", [] { throw std::runtime_error("数据库连接超时"); }));
    agg.submit(SafeTask("任务C", [] { std::cout << "  任务C: 成功\n"; }));
    agg.submit(SafeTask("任务D", [] { throw std::logic_error("参数校验失败"); }));

    if (agg.has_errors()) {
        std::cerr << "  以下任务执行失败:\n";
        agg.report();
    }
}

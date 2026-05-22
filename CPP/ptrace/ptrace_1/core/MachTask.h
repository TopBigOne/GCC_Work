#pragma once
/**
 * MachTask.h
 * 设计模式：RAII
 *   - 构造时调用 task_for_pid 获取子进程 Mach task port
 *   - 析构时自动释放 mach port，防止 port 泄漏
 */
#include <mach/mach.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

class MachTask {
public:
    explicit MachTask(pid_t pid) {
        kern_return_t kr = task_for_pid(mach_task_self(), pid, &task_);
        if (kr != KERN_SUCCESS) {
            throw std::runtime_error(
                std::string("task_for_pid failed: ") + mach_error_string(kr) +
                "\n  提示：确保是父子进程关系，或添加 com.apple.security.cs.debugger entitlement");
        }
    }

    ~MachTask() {
        if (task_ != MACH_PORT_NULL)
            mach_port_deallocate(mach_task_self(), task_);
    }

    // 禁止拷贝，允许移动
    MachTask(const MachTask&) = delete;
    MachTask& operator=(const MachTask&) = delete;

    MachTask(MachTask&& other) noexcept : task_(other.task_) {
        other.task_ = MACH_PORT_NULL;
    }

    task_t get() const { return task_; }

private:
    task_t task_ = MACH_PORT_NULL;
};

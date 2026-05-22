#pragma once
#include "IDemo.h"

/**
 * DemoMemory
 * 演示父进程读取/修改子进程内存：
 *   - 子进程有一个全局字符串缓冲区
 *   - fork 后父子进程共享相同虚拟地址（COW 隔离物理页）
 *   - 父进程：读取子进程的字符串 → 修改它 → 放行子进程
 *   - 子进程：读取缓冲区，验证父进程写入的新值
 */
class DemoMemory : public IDemo {
public:
    std::string name()        const override { return "ReadWriteMemory"; }
    std::string description() const override {
        return "读取子进程全局变量内容，再向其写入新数据，子进程验证修改生效";
    }
    void run() override;
};

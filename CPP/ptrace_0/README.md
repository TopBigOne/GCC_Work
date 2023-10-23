### ptrace 防调试
* [linux-沙盒入门，ptrace从0到1](https://cloud.tencent.com/developer/article/1799705)

```c++
#include <sys/ptrace.h>       
long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);
```
一共有四个参数：

* request: 表示要执行的操作类型。//反调试会用到PT_DENY_ATTACH，调试会用到PTRACE_ATTACH
* pid:   要操作的目标进程ID
* addr: 要监控的目标内存地址
* data: 保存读取出或者要写入的数据

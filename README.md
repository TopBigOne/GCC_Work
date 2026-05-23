# GCC_Work — C/C++ & Linux 学习知识点目录

> 涵盖 C/C++ 语言特性、Linux 系统编程、并发多线程、网络编程等方向，53+ 个 CMake 子项目。
> 点击每个知识点条目可跳转至对应源码文件。

---

## 目录

- [一、C++ 基础语法](#一c-基础语法)
- [二、C++ 面向对象](#二c-面向对象)
- [三、C++ 模板](#三c-模板)
- [四、C++ 异常处理](#四c-异常处理)
- [五、C++11 — 类型系统增强](#五c11--类型系统增强)
- [六、C++11 — 移动语义与资源管理](#六c11--移动语义与资源管理)
- [七、C++11 — 函数式编程](#七c11--函数式编程)
- [八、C++11/17 — 标准库新容器与工具类](#八c1117--标准库新容器与工具类)
- [九、C++11 — 智能指针](#九c11--智能指针)
- [十、C++ 标准库容器](#十c-标准库容器)
- [十一、C++20 — 协程（Coroutines）](#十一c20--协程coroutines)
- [十二、内存与指针](#十二内存与指针)
- [十三、设计模式](#十三设计模式)
- [十四、C 标准库 I/O](#十四c-标准库-io)
- [十五、Linux 系统编程](#十五linux-系统编程)
- [十六、并发与多线程](#十六并发与多线程)
- [十七、网络编程（Socket）](#十七网络编程socket)
- [十八、加密与压缩](#十八加密与压缩)
- [十九、第三方库集成](#十九第三方库集成)

---

## 一、C++ 基础语法

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| 变量声明与初始化 | [declare_variable/main.cpp](CPP/declare_variable/main.cpp) | 各类型变量声明、零初始化、列表初始化 |
| `const` 常量 | [const/main.cpp](CPP/const/main.cpp) | 顶层/底层 const，const 引用，指向常量的指针 |
| `constexpr` 编译期常量 | [constexpr/main.cpp](CPP/constexpr/main.cpp) | constexpr 函数、变量，与 const 的区别 |
| `static` 静态变量与函数 | [static/main.c](CPP/static/main.c) | 局部静态、文件静态、类静态成员 |
| `extern` 外部链接 | [key_extern/main.cpp](CPP/key_extern/main.cpp) | 跨文件共享变量与函数声明 |
| `union` 联合体 | [key_union/main.cpp](CPP/key_union/main.cpp) | 内存复用，union 成员访问 |
| `struct` 结构体 | [struct/main.cpp](CPP/struct/main.cpp) | 结构体定义、成员初始化、嵌套结构 |
| `enum` 枚举 | [test_enum/main.cpp](CPP/test_enum/main.cpp) | 传统 enum 与 enum class 对比 |
| 位运算 | [test_bit_operation/main.cpp](CPP/test_bit_operation/main.cpp) | 与/或/异或/移位，位掩码实战 |
| `assert` 断言 | [assert/main.cpp](CPP/assert/main.cpp) | 运行期断言，static_assert 编译期检查 |
| `noexcept` 异常规范 | [noexcept/main.cpp](CPP/noexcept/main.cpp) | noexcept 修饰符，移动语义配合 noexcept |
| `alignas` 内存对齐 | [alignas/main.cpp](CPP/alignas/main.cpp) | 强制对齐，alignof 查询对齐要求 |
| `explicit` 关键字 | [K_W_explicit/main.cpp](CPP/K_W_explicit/main.cpp) · [explicit/main.cpp](CPP/explicit/main.cpp) | 禁止隐式转换，explicit 构造函数与转换函数 |
| 预处理指令 | [Preprocessor/main.c](CPP/Preprocessor/main.c) | `#ifdef`/`#ifndef` 条件编译，防重复包含 `doubincl.h` |
| `sprintf` 字符串格式化 | [sprintf/main.cpp](CPP/sprintf/main.cpp) | sprintf/snprintf 安全用法 |
| `stringstream` 字符串流 | [stringstream/main.cpp](CPP/stringstream/main.cpp) | istringstream/ostringstream 字符串转换 |
| 数组越界检测 | [test_array_out_of_bounds/main.cpp](CPP/test_array_out_of_bounds/main.cpp) | 越界行为与 sanitizer 检测 |
| 循环中 return 行为 | [return_in_for_loop/main.c](CPP/return_in_for_loop/main.c) | 各类循环中 return/break/continue 语义 |
| 头文件模块化 | [test_head_file/main.c](test_head_file/main.c) · [my_print.h](test_head_file/my_print.c) | .h 声明 + .c 实现分离 |
| 无头文件单文件写法 | [without_head_file/main.c](without_head_file/main.c) | 所有代码集中在一个 .c 文件 |

---

## 二、C++ 面向对象

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| 抽象类与纯虚函数 | [AbstractClass/main.cpp](CPP/AbstractClass/main.cpp) · [AbstractClass.h](CPP/AbstractClass/AbstractClass.h) | 纯虚函数定义，接口约定，多态调用 |
| 具体类实现 | [ConcreteClass.cpp](CPP/AbstractClass/ConcreteClass.cpp) | 继承抽象类，override 实现 |
| 构造函数与生命周期 | [test_main_and_constructor/main.cpp](CPP/test_main_and_constructor/main.cpp) | 构造/析构顺序，成员初始化列表 |
| 类的内存布局 | [memory_of_class/main.cpp](CPP/memory_of_class/main.cpp) · [A.h](CPP/memory_of_class/A.h) | 虚表指针、成员排布、sizeof 类 |
| `explicit` 构造函数 | [ExplicitClass.h](CPP/explicit/ExplicitClass.h) · [NormalClass.h](CPP/explicit/NormalClass.h) | explicit 与隐式转换对比 |
| 回调机制 | [CPP_Callback/Callback.h](CPP/CPP_Callback/Callback.h) · [Caller.h](CPP/CPP_Callback/Caller.h) · [Receiver.h](CPP/CPP_Callback/Receiver.h) | 观察者模式，注册/触发回调 |
| swap 实现与重载 | [test_Swap/main.cpp](CPP/test_Swap/main.cpp) | 自定义 swap，ADL 查找 |

---

## 三、C++ 模板

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| 函数模板基础 | [template/main.cpp](CPP/template/main.cpp) | 模板参数推导，显式特化 |
| 模板进阶 | [template_demo_1/main.cpp](CPP/template_demo_1/main.cpp) | 非类型模板参数，模板偏特化 |

---

## 四、C++ 异常处理

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `try` / `catch` / `throw` | [try_catch/main.cpp](CPP/try_catch/main.cpp) | 异常抛出与捕获，自定义异常类 |

---

## 五、C++11 — 类型系统增强

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `decltype` 类型推导 | [decltype/main.cpp](CPP/decltype/main.cpp) | decltype(expr) 推导，decltype(auto)，与 auto 对比 |
| `std::string_view` | [std_string_view/main.cpp](CPP/std_string_view/main.cpp) | 零拷贝字符串视图，substr/find，避免分配 |
| `std::string` 常用操作 | [std_string/main.cpp](CPP/std_string/main.cpp) | find/replace/substr/append/compare |
| `std::tuple` 元组 | [tuple/main.cpp](CPP/tuple/main.cpp) | make_tuple/get/tie，结构化绑定 |

---

## 六、C++11 — 移动语义与资源管理

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `std::move` 移动语义 | [std_move/main.cpp](CPP/std_move/main.cpp) | 右值引用，移动构造/赋值，避免深拷贝 |
| `std::forward` 完美转发 | [std_forward/main.cpp](CPP/std_forward/main.cpp) · [forward_demos.h](CPP/std_forward/forward_demos.h) | 万能引用，forward 保留值类别 |
| `std::ref` / `std::cref` | [std_ref/main.cpp](CPP/std_ref/main.cpp) | 引用包装器，传引用给 bind/thread |

---

## 七、C++11 — 函数式编程

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| Lambda 基础语法 | [Lambda/day_01/BaseLambda.cpp](CPP/Lambda/day_01/BaseLambda.cpp) · [BaseLambda.h](CPP/Lambda/day_01/BaseLambda.h) | `[]` / `[=]` / `[&]` 捕获，返回类型推导 |
| Lambda 捕获列表详解 | [Lambda/day_01/CaptureList.cpp](CPP/Lambda/day_01/CaptureList.cpp) · [CaptureList.h](CPP/Lambda/day_01/CaptureList.h) | 按值/按引用/初始化捕获，mutable lambda |
| `std::function` 函数包装器 | [std_function/main.cpp](CPP/std_function/main.cpp) | 包装函数/lambda/成员函数，类型擦除 |
| `std::bind` 参数绑定 | [std_bind/main.cpp](CPP/std_bind/main.cpp) | 绑定参数，占位符 `_1`/`_2`，与 lambda 对比 |

---

## 八、C++11/17 — 标准库新容器与工具类

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `std::optional` 可选值 | [std_optional/main.cpp](CPP/std_optional/main.cpp) | has_value/value/value_or，代替 nullptr 哨兵 |
| `std::variant` 类型安全联合 | [std_variant/main.cpp](CPP/std_variant/main.cpp) | std::get/std::visit，替代 union |
| `std::array` 固定大小数组 | [std_array/main.cpp](CPP/std_array/main.cpp) · [TestArray.h](CPP/std_array/TestArray.h) | 栈上数组，size/at/fill，与 C 数组对比 |
| `std::unordered_multimap` | [std_unordered_multimap/main.cpp](CPP/std_unordered_multimap/main.cpp) | 哈希桶，equal_range，一键多值 |
| `std::promise` 异步值传递 | [std_promise/demo_1/main.cpp](CPP/std_promise/demo_1/main.cpp) | promise/future，跨线程传值与异常 |

---

## 九、C++11 — 智能指针

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `std::shared_ptr` 共享所有权 | [std/shared_ptr/main.cpp](CPP/std/shared_ptr/main.cpp) | 引用计数，循环引用问题 |
| `std::make_shared` | [make_shared/main.cpp](CPP/make_shared/main.cpp) · [Sunday.h](CPP/make_shared/Sunday.h) | 一次分配控制块+对象，性能优于 new |
| `std::unique_ptr` 独占所有权 | [unique_ptr/main.cpp](CPP/unique_ptr/main.cpp) | 移动语义转移所有权，自定义 deleter |
| 内存分配器 | [allocator/main.cpp](CPP/allocator/main.cpp) | std::allocator，自定义分配器接口 |

---

## 十、C++ 标准库容器

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `std::forward_list` 单链表 | [forward_list/main.cpp](CPP/forward_list/main.cpp) | push_front/insert_after，O(1) 头插 |
| `std::list` 双向链表 | [list_push_front/main.cpp](CPP/list_push_front/main.cpp) | push_front/back，迭代器稳定性 |
| `std::map` 有序映射 | [map/main.cpp](CPP/map/main.cpp) · [map/case_kv/CaseKV.h](CPP/map/case_kv/CaseKV.h) | insert/find/erase，红黑树，按 key 遍历 |
| `std::vector` 动态数组 | [modern-cpp/4_array/Test_vector.cpp](modern-cpp/4_array/Test_vector.cpp) | push_back/reserve/emplace_back |

---

## 十一、C++20 — 协程（Coroutines）

目录：[C20_Coroutines/LearnCoroutine/](C20_Coroutines/LearnCoroutine/)

| 模块 | 源码 | 核心知识点 |
|------|------|-----------|
| 先决概念 | [module0_prerequisites/prerequisites.h](C20_Coroutines/LearnCoroutine/module0_prerequisites/prerequisites.h) | 函数调用栈、挂起/恢复的底层机制 |
| 基础语法 | [module1_basics/basics.h](C20_Coroutines/LearnCoroutine/module1_basics/basics.h) | `co_await` / `co_yield` / `co_return` |
| Promise 对象 | [module2_promise/promise.h](C20_Coroutines/LearnCoroutine/module2_promise/promise.h) | promise_type，initial/final_suspend，return_value |
| Generator 模式 | [module3_generator/generator.h](C20_Coroutines/LearnCoroutine/module3_generator/generator.h) | 惰性序列生成，co_yield 迭代 |
| Awaitable 协议 | [module4_awaitable/awaitable.h](C20_Coroutines/LearnCoroutine/module4_awaitable/awaitable.h) | await_ready/await_suspend/await_resume 三件套 |
| Task 封装 | [module5_task/task.h](C20_Coroutines/LearnCoroutine/module5_task/task.h) | 异步 Task 类型，链式 co_await |
| 实战：调度器 | [module6_realworld/scheduler.h](C20_Coroutines/LearnCoroutine/module6_realworld/scheduler.h) | 协程调度器，任务队列驱动 |
| 实战：事件系统 | [module6_realworld/event.h](C20_Coroutines/LearnCoroutine/module6_realworld/event.h) | 事件等待，协程唤醒 |
| 实战：Channel | [module6_realworld/channel.h](C20_Coroutines/LearnCoroutine/module6_realworld/channel.h) | 生产者/消费者，有界 channel |
| 综合示例 | [module6_realworld/realworld.h](C20_Coroutines/LearnCoroutine/module6_realworld/realworld.h) | 多协程协同运行 |
| 入口 main | [LearnCoroutine/main.cpp](C20_Coroutines/LearnCoroutine/main.cpp) | 各模块调用入口 |

---

## 十二、内存与指针

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| 函数指针 | [pointer/1_func_pointer/main.cpp](CPP/pointer/1_func_pointer/main.cpp) · [CallbackFunction.h](CPP/pointer/1_func_pointer/CallbackFunction.h) | 函数指针声明、传参、回调注册 |
| 栈指针 vs 堆指针 | [pointer/2_stackPointer_and_heapPointer/main.cpp](CPP/pointer/2_stackPointer_and_heapPointer/main.cpp) | 栈/堆地址特征，new/delete 生命周期 |
| 指针数组 | [pointer/3_pointer_array/main.cpp](CPP/pointer/3_pointer_array/main.cpp) | 指针数组 vs 数组指针，二维数组访问 |
| 字符指针操作 | [pointer/4_point_char/main.c](CPP/pointer/4_point_char/main.c) | C 字符串，字符指针遍历与修改 |
| 字符串指针进阶 | [pointer/10-avstring/main.c](CPP/pointer/10-avstring/main.c) | av 字符串处理，指针算术 |
| `memcpy` 内存拷贝 | [memcpy/main.c](CPP/memcpy/main.c) | 拷贝语义，与 memmove 重叠区别 |
| 内存对齐详解 | [memory_aligin/main.cpp](memory_aligin/main.cpp) · [test_struct.c](memory_aligin/test_struct.c) | alignof，struct padding，`#pragma pack` |

---

## 十三、设计模式

目录：[CPP/DesignPatterns/1_singleton/](CPP/DesignPatterns/1_singleton/)

| 模式 | 源码 | 说明 |
|------|------|------|
| 饿汉式单例 | [eager_singleton.h](CPP/DesignPatterns/1_singleton/eager_singleton.h) | 静态成员，类加载时初始化，天然线程安全 |
| 懒汉式单例 | [lazy_singleton.h](CPP/DesignPatterns/1_singleton/lazy_singleton.h) | 首次调用时初始化，需加锁保护 |
| Meyers 单例 | [meyers_singleton.h](CPP/DesignPatterns/1_singleton/meyers_singleton.h) | 局部静态变量，C++11 保证线程安全 |
| 双重检查锁单例 | [thread_safe_singleton.h](CPP/DesignPatterns/1_singleton/thread_safe_singleton.h) | `std::once_flag` + `std::call_once` |
| 单例对比 main | [DesignPatterns/1_singleton/main.cpp](CPP/DesignPatterns/1_singleton/main.cpp) | 四种实现对比调用 |

---

## 十四、C 标准库 I/O

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `fopen` / `fclose` 文件打开 | [fopen/main.cpp](CPP/fopen/main.cpp) | 模式字符串（r/w/a/rb/wb），错误检查 |
| `fgets` / `fputs` 文本读写 | [fgets/main.c](CPP/fgets/main.c) · [fputs/main.c](CPP/fputs/main.c) · [12_4_fgets_fputs/main.cpp](CPP/12_4_fgets_fputs/main.cpp) | 行级文本读写，EOF 处理 |
| `fread` / `fwrite` 二进制读写 | [fread/main.c](CPP/fread/main.c) · [12_5_fread_fwrite/main.cpp](CPP/12_5_fread_fwrite/main.cpp) | 块级二进制 I/O，返回值检查 |
| `fseek` / `ftell` 文件定位 | [fseek/main.c](CPP/fseek/main.c) · [feek_demo_1/main.cpp](CPP/feek_demo_1/main.cpp) | SEEK_SET/CUR/END，随机访问文件 |
| `fstat` 文件状态 | [fstat/main.c](CPP/fstat/main.c) | 获取文件大小、权限、修改时间 |
| `fprintf` 格式化写入 | [fprintf/main.c](CPP/fprintf/main.c) | 向文件写格式化字符串 |
| 文件追加模式 | [13_5_append/main.c](CPP/13_5_append/main.c) | "a" 模式，追加写入不覆盖原内容 |
| FILE 指针管理 | [FILE_Pointer/main.cpp](CPP/FILE_Pointer/main.cpp) | FILE 结构，缓冲区，fflush |
| `getchar` / `putchar` | [getchar/main.cpp](CPP/getchar/main.cpp) · [putchar/main.c](CPP/putchar/main.c) | 单字符 I/O，stdin/stdout 操作 |
| `strchr` 字符串查找 | [strchr/main.c](CPP/strchr/main.c) | 在字符串中查找字符，返回指针 |

---

## 十五、Linux 系统编程

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| 信号处理 `sigaction` | [linux_sigaction/main.cpp](linux_sigaction/main.cpp) | 注册信号处理器，sa_handler，`raise()` 发送信号 |
| `lseek` 文件描述符定位 | [lseek/main.c](CPP/lseek/main.c) | 低级 fd 偏移，与 fseek 区别 |
| `dup2` 重定向文件描述符 | [dup2/main.cpp](CPP/dup2/main.cpp) | 输出重定向，管道，stdout → 文件 |
| `ptrace` 基础：进程追踪 | [ptrace/ptrace_0/main.cpp](CPP/ptrace/ptrace_0/main.cpp) | PTRACE_ATTACH/DETACH，读取寄存器 |
| `ptrace` 进阶：调试器实现 | [ptrace/ptrace_1/main.cpp](CPP/ptrace/ptrace_1/main.cpp) | 完整调试器框架 |
| — 断点管理 | [ptrace_1/core/Breakpoint.h](CPP/ptrace/ptrace_1/core/Breakpoint.h) · [Breakpoint.cpp](CPP/ptrace/ptrace_1/core/Breakpoint.cpp) | INT3 软件断点设置与恢复 |
| — 调试器主控 | [ptrace_1/core/Debugger.h](CPP/ptrace/ptrace_1/core/Debugger.h) · [Debugger.cpp](CPP/ptrace/ptrace_1/core/Debugger.cpp) | 命令行调试器，run/continue/step |
| — 进程控制 | [ptrace_1/core/ProcessCtrl.h](CPP/ptrace/ptrace_1/core/ProcessCtrl.h) | waitpid，信号处理，进程状态 |
| — 内存访问 | [ptrace_1/core/MemoryAccess.h](CPP/ptrace/ptrace_1/core/MemoryAccess.h) | PTRACE_PEEKDATA/POKEDATA 读写内存 |
| — 寄存器访问 | [ptrace_1/core/RegisterAccess.h](CPP/ptrace/ptrace_1/core/RegisterAccess.h) | 读写 x86_64 通用寄存器 |
| — 反调试检测 demo | [ptrace_1/demos/DemoAntiDebug.h](CPP/ptrace/ptrace_1/demos/DemoAntiDebug.h) | 检测自身是否被调试 |
| — 单步执行 demo | [ptrace_1/demos/DemoSingleStep.h](CPP/ptrace/ptrace_1/demos/DemoSingleStep.h) | PTRACE_SINGLESTEP 逐指令执行 |
| — 内存读写 demo | [ptrace_1/demos/DemoMemory.h](CPP/ptrace/ptrace_1/demos/DemoMemory.h) | 向目标进程注入/读取数据 |
| 音视频 I/O（avio） | [21-avio-main-luo/main.c](CPP/21-avio-main-luo/main.c) · [21-avio-write-luo/main.c](CPP/21-avio-write-luo/main.c) | FFmpeg AVIOContext 读写封装 |

---

## 十六、并发与多线程

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| `std::thread` — 函数指针创建 | [create_thread/Case_1_function_Pointer.cpp](c11_thread/create_thread/Case_1_function_Pointer.cpp) · [.h](c11_thread/create_thread/Case_1_function_Pointer.h) | 最基础的线程创建方式 |
| `std::thread` — 三种创建方式汇总 | [create_thread/main.cpp](c11_thread/create_thread/main.cpp) | 函数指针/函数对象/Lambda 三种入口对比 |
| Pthreads 基础 | [pthread_base_ability/main.c](c11_thread/pthread_base_ability/main.c) | pthread_create/join/detach，属性设置 |
| `pthread_cancel` 线程取消 | [pthread_cancel/main.cpp](c11_thread/pthread_cancel/main.cpp) | 取消点，PTHREAD_CANCEL_DEFERRED |
| 线程优雅停止（标志位法） | [stop_thread/main.cpp](c11_thread/stop_thread/main.cpp) | atomic bool 退出标志，避免强杀 |
| 线程优雅停止（扩展封装） | [stop_thread_2/main.cpp](c11_thread/stop_thread_2/main.cpp) · [ThreadExtension.h](c11_thread/stop_thread_2/ThreadExtension.h) | RAII 封装线程生命周期 |
| 线程池实现 | [cpp_11_thtread_poll/ThreadPool.h](c11_thread/cpp_11_thtread_poll/ThreadPool.h) | 任务队列，condition_variable，worker 线程组 |
| `std::promise` / `std::future` | [std_promise/demo_1/main.cpp](CPP/std_promise/demo_1/main.cpp) | 跨线程单次值传递，get() 阻塞等待 |

---

## 十七、网络编程（Socket）

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| TCP Server 实现 | [ISocket/ISocket_Server/main.cpp](Interview/ISocket/ISocket_Server/main.cpp) | socket → bind → listen → accept → recv/send 完整流程 |
| TCP Client 实现 | [ISocket/ISocket_Client/main.cpp](Interview/ISocket/ISocket_Client/main.cpp) | socket → connect → send/recv → close |

---

## 十八、加密与压缩

| 知识点 | 源码入口 | 说明 |
|--------|----------|------|
| OpenSSL EVP 对称加密 | [Crypto_Demo/TestEvp.h](CPP/Crypto_Demo/TestEvp.h) · [TestEvp.cpp](CPP/Crypto_Demo/TestEvp.cpp) | EVP_EncryptInit/Update/Final，AES-CBC |
| Zlib 压缩 / 解压 | [Crypto_Demo/TestZlib.h](CPP/Crypto_Demo/TestZlib.h) · [TestZlib.cpp](CPP/Crypto_Demo/TestZlib.cpp) | deflate/inflate，z_stream 使用 |
| MD5 哈希 | [Md_Five/main.cpp](CPP/Md_Five/main.cpp) | MD5 计算，二进制 → 十六进制字符串 |
| 字符串混淆（Proguard 风格） | [Crypto_Demo/handle_string/Proguard.h](CPP/Crypto_Demo/handle_string/Proguard.h) · [OnlyEncryptString.h](CPP/Crypto_Demo/handle_string/OnlyEncryptString.h) | 编译期字符串加密，防静态扫描 |
| 流式加解密管理器 | [Crypto_Demo/FlowManager.h](CPP/Crypto_Demo/FlowManager.h) · [FlowManager.cpp](CPP/Crypto_Demo/FlowManager.cpp) | 封装流式加解密，分块处理大文件 |
| 加密综合入口 | [Crypto_Demo/main.cpp](CPP/Crypto_Demo/main.cpp) | 各功能集成演示 |

---

## 十九、第三方库集成

| 库 | 知识点 | 源码入口 |
|----|--------|----------|
| **SDL2** | 图形/游戏开发环境搭建 | [Test_sdl2_Env/](Test_sdl2_Env/) |
| **Boost.Thread** | Boost 线程库，对比 `std::thread` | [test_env_boost/TestBoostThread.h](test_env_boost/TestBoostThread.h) |
| **OpenSSL** | EVP 加密接口 | [Crypto_Demo/TestEvp.h](CPP/Crypto_Demo/TestEvp.h) |
| **Zlib** | deflate/inflate 数据压缩 | [Crypto_Demo/TestZlib.h](CPP/Crypto_Demo/TestZlib.h) |

---

## 参考资料

| 文档 | 路径 |
|------|------|
| CMake 权威指南 | [DOC/Professional CMake A Practical Guide by Craig Scott.pdf](DOC/Professional%20CMake%20A%20Practical%20Guide%20by%20Craig%20Scott.pdf) |
| 正点原子嵌入式 Linux C 应用编程指南 | [DOC/【正点原子】I.MX6U嵌入式Linux C应用编程指南V1.4.pdf](DOC/【正点原子】I.MX6U嵌入式Linux C应用编程指南V1.4.pdf) |
| Modern C++ 教程（中文） | [modern-cpp/modern-cpp-tutorial-zh-cn.pdf](modern-cpp/modern-cpp-tutorial-zh-cn%20(1).pdf) |

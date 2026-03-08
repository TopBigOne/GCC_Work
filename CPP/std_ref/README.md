# C++ std::ref 学习文档

## 概述

`std::ref` 是 C++11 引入的一个重要工具，**用来创建对某个对象的引用包装（reference wrapper）。**
它解决了在需要传递引用但接口只接受值传递的场景中保持引用语义的问题。

## 为什么需要 std::ref？

在某些情况下，我们希望传递引用，但是函数或模板只接受值传递。比如：
- `std::thread` 构造函数
- `std::bind`
- 标准容器和算法

这些场景下直接传递变量会导致值拷贝，而不是引用传递。

## 基本语法

```cpp
#include <functional>

int x = 10;
auto ref_x = std::ref(x);        // 创建引用包装
std::reference_wrapper<int> ref_x2 = std::ref(x);  // 等价写法
```

## 实际应用示例

### 1. 在 std::thread 中使用

```cpp
#include <iostream>
#include <thread>

void foo1(int *x) {
    *x += 10;
}

void test_1() {
    int a = 5;
    int *p_a = &a;
    std::thread t1(foo1, p_a);  // 传递指针
    t1.join();
    std::cout << "case 1 : a :" << a << std::endl;  // 输出: 15
}

void foo2(int &x) {
    x += 11;
}

void test_2() {
    int b = 5;
    std::thread t2(foo2, std::ref(b));  // 使用 std::ref 传递引用
    t2.join();
    std::cout << "case 2 : b :" << b << std::endl;  // 输出: 16
}
```

**对比分析：**
- `test_1()`: 使用指针传递，可以修改原始值
- `test_2()`: 使用 `std::ref` 传递引用，同样可以修改原始值

如果在 `test_2()` 中不使用 `std::ref`：
```cpp
std::thread t2(foo2, b);  // 错误！会尝试值传递给引用参数
```
这会导致编译错误，因为 `foo2` 期望引用参数，但 `std::thread` 会尝试拷贝 `b`。

### 2. 在 std::bind 中使用

```cpp
#include <functional>
#include <iostream>

void modify_value(int& x) {
    x += 100;
}

void test_bind() {
    int value = 10;

    // 错误的方式 - 值传递
    auto func1 = std::bind(modify_value, value);
    // func1();  // 编译错误！

    // 正确的方式 - 引用传递
    auto func2 = std::bind(modify_value, std::ref(value));
    func2();
    std::cout << "value after bind: " << value << std::endl;  // 110
}
```

### 3. 在容器中存储引用

```cpp
#include <vector>
#include <functional>

void test_container() {
    int a = 1, b = 2, c = 3;

    // 使用 reference_wrapper 在容器中存储引用
    std::vector<std::reference_wrapper<int>> refs = {
        std::ref(a), std::ref(b), std::ref(c)
    };

    // 修改引用包装的值
    for (auto& ref : refs) {
        ref.get() *= 10;  // 使用 .get() 访问原始引用
    }

    std::cout << "a=" << a << ", b=" << b << ", c=" << c << std::endl;  // a=10, b=20, c=30
}
```

## std::reference_wrapper 的主要方法

```cpp
int x = 42;
std::reference_wrapper<int> ref = std::ref(x);

// 1. get() - 获取原始引用
int& original = ref.get();

// 2. 隐式转换为引用类型
int& another_ref = ref;

// 3. operator() - 如果包装的是可调用对象
// 例如函数对象或函数指针
```

## 重要注意事项

### ⚠️ 不要对临时变量使用 std::ref

```cpp
#include <iostream>
#include <functional>

void foo(std::reference_wrapper<int> r) {
    std::cout << r.get() << std::endl;
}

int main() {
    int a = 200;
    foo(std::ref(a));    // ✅ 正确：a 是持久变量
    foo(std::ref(100));  // ❌ 危险：100 是临时变量，会导致悬挂引用
}
```

**为什么？**
- `100` 是临时变量，在表达式结束后立即销毁
- `std::ref(100)` 创建的引用包装会指向已销毁的内存
- 使用这样的引用包装会导致未定义行为

### 生命周期管理

```cpp
std::reference_wrapper<int> create_dangerous_ref() {
    int local = 42;
    return std::ref(local);  // ❌ 危险：返回局部变量的引用
}  // local 在这里被销毁

void safe_usage() {
    int persistent = 42;
    auto safe_ref = std::ref(persistent);  // ✅ 安全：persistent 有足够长的生命周期
    // ... 使用 safe_ref
}
```

## 与普通引用的对比

| 特性 | 普通引用 (`int&`) | `std::reference_wrapper<int>` |
|------|-------------------|------------------------------|
| 可重新绑定 | ❌ | ✅ |
| 可存储在容器中 | ❌ | ✅ |
| 可作为函数返回值 | ⚠️ (需小心) | ✅ |
| 可默认构造 | ❌ | ❌ |
| 可拷贝 | ❌ | ✅ |

```cpp
int x = 1, y = 2;

// 普通引用
int& ref1 = x;
// ref1 = y;  // 这是赋值操作，不是重新绑定

// reference_wrapper
std::reference_wrapper<int> ref2 = std::ref(x);
ref2 = std::ref(y);  // ✅ 可以重新绑定到 y
```

## 常见使用场景总结

1. **多线程编程**：向 `std::thread` 传递引用参数
2. **函数绑定**：与 `std::bind` 一起使用保持引用语义
3. **容器存储**：在标准容器中存储引用
4. **算法使用**：向标准算法传递引用参数
5. **回调函数**：在回调机制中保持引用语义

## 编译和运行

```bash
g++ -std=c++11 main.cpp -o std_ref
./std_ref
```

## 相关函数

- `std::ref()`: 创建引用包装
- `std::cref()`: 创建 const 引用包装
- `std::reference_wrapper<T>`: 引用包装类模板

## 总结

`std::ref` 是 C++ 现代编程中的重要工具，它让我们能够在值传递的接口中保持引用语义。正确使用它可以避免不必要的拷贝，提高程序性能，但同时也要注意避免创建悬挂引用的陷阱。

以下是一套完整的 `std::function` 学习案例代码，覆盖 **基础用法**、**高级场景** 和 **实际应用**，每个案例均有详细说明：

---

### **案例 1：基础用法**
#### 1.1 定义和调用
```cpp
#include <iostream>
#include <functional>

// 普通函数
int add(int a, int b) { return a + b; }

// 仿函数（函数对象）
struct Multiply {
    int operator()(int a, int b) { return a * b; }
};

int main() {
    // 定义 std::function 类型
    std::function<int(int, int)> func;

    // 绑定普通函数
    func = add;
    std::cout << "Add: " << func(3, 4) << std::endl;  // 输出 7

    // 绑定仿函数对象
    func = Multiply{};
    std::cout << "Multiply: " << func(3, 4) << std::endl;  // 输出 12

    // 绑定 Lambda 表达式
    func = [](int a, int b) { return a - b; };
    std::cout << "Subtract: " << func(10, 5) << std::endl;  // 输出 5

    // 绑定带捕获的 Lambda
    int offset = 2;
    func = [offset](int a, int b) { return (a + b) / offset; };
    std::cout << "Average: " << func(10, 6) << std::endl;  // 输出 8
}
```

#### 1.2 成员函数
```cpp
#include <functional>

class Calculator {
public:
    int divide(int a, int b) { return a / b; }
    static int mod(int a, int b) { return a % b; }
};

int main() {
    Calculator calc;

    // 绑定非静态成员函数（需要对象实例）
    std::function<int(int, int)> func1 = std::bind(
        &Calculator::divide, &calc, std::placeholders::_1, std::placeholders::_2
    );
    std::cout << "Divide: " << func1(10, 3) << std::endl;  // 输出 3

    // 绑定静态成员函数（与普通函数相同）
    std::function<int(int, int)> func2 = &Calculator::mod;
    std::cout << "Mod: " << func2(10, 3) << std::endl;  // 输出 1
}
```

---

### **案例 2：高级用法**
#### 2.1 存储回调到容器
```cpp
#include <vector>
#include <functional>

int main() {
    std::vector<std::function<void()>> tasks;

    // 添加不同任务到容器
    tasks.push_back([]{ std::cout << "Task 1\n"; });
    tasks.push_back([]{ std::cout << "Task 2\n"; });

    int x = 5;
    tasks.push_back([x] { std::cout << "Task 3: " << x << " \n"; });

    // 执行所有任务
    for (auto& task : tasks) {
        task();
    }
}
```

#### 2.2 作为函数参数（回调机制）
```cpp
#include <functional>
#include <vector>

// 定义一个数据处理函数，接受回调
void processData(const std::vector<int>& data, std::function<void(int)> callback) {
    for (int num : data) {
        callback(num * 2);  // 对每个元素应用回调
    }
}

int main() {
    std::vector<int> numbers = {1, 2, 3, 4};

    // 使用 Lambda 作为回调
    processData(numbers, [](int x) {
        std::cout << "Processed: " << x << std::endl;
    });

    // 输出：
    // Processed: 2
    // Processed: 4
    // Processed: 6
    // Processed: 8
}
```

---

### **案例 3：实际应用**
#### 3.1 事件系统（观察者模式）
```cpp
#include <functional>
#include <vector>
#include <string>

class EventSystem {
private:
    std::vector<std::function<void(const std::string&)>> listeners;

public:
    // 注册事件监听器
    void addListener(std::function<void(const std::string&)> listener) {
        listeners.push_back(listener);
    }

    // 触发事件
    void triggerEvent(const std::string& message) {
        for (auto& listener : listeners) {
            listener(message);
        }
    }
};

int main() {
    EventSystem eventSystem;

    // 添加监听器 1
    eventSystem.addListener([](const std::string& msg) {
        std::cout << "Listener 1: " << msg << std::endl;
    });

    // 添加监听器 2（带捕获）
    std::string prefix = "Log> ";
    eventSystem.addListener([prefix](const std::string& msg) {
        std::cout << prefix << msg << std::endl;
    });

    // 触发事件
    eventSystem.triggerEvent("Hello World");
    // 输出：
    // Listener 1: Hello World
    // Log> Hello World
}
```

#### 3.2 策略模式
```cpp
#include <functional>
#include <vector>

class Sorter {
private:
    std::function<bool(int, int)> comparator;

public:
    // 设置比较策略
    void setComparator(std::function<bool(int, int)> comp) {
        comparator = comp;
    }

    // 使用策略排序
    void sort(std::vector<int>& data) {
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = i+1; j < data.size(); ++j) {
                if (comparator(data[i], data[j])) {
                    std::swap(data[i], data[j]);
                }
            }
        }
    }
};

int main() {
    Sorter sorter;
    std::vector<int> numbers = {5, 2, 9, 1, 5};

    // 设置升序策略
    sorter.setComparator([](int a, int b) { return a > b; });
    sorter.sort(numbers);
    // numbers 变为 {1, 2, 5, 5, 9}

    // 设置降序策略
    sorter.setComparator([](int a, int b) { return a < b; });
    sorter.sort(numbers);
    // numbers 变为 {9, 5, 5, 2, 1}
}
```

---

### **案例 4：错误用法示例**
```cpp
#include <functional>

class MyClass {
public:
    void memberFunc() { /* ... */ }
};

int main() {
    std::function<void()> func;

    // 错误：直接绑定非静态成员函数（缺少对象实例）
    // func = &MyClass::memberFunc;  // 编译错误

    // 正确：使用 std::bind 绑定对象
    MyClass obj;
    func = std::bind(&MyClass::memberFunc, &obj);
}
```

---

### **总结**
通过这些案例，你可以看到 `std::function` 如何：
1. 统一处理函数、Lambda、仿函数等不同类型的可调用对象
2. 实现回调机制、事件系统、策略模式等高级设计模式
3. 存储和管理多个回调函数

关键点：
- 使用 `std::bind` 绑定成员函数时需要提供对象实例
- 优先用 Lambda 简化代码（尤其是需要捕获局部变量时）
- 注意 `std::function` 的性能开销（高频调用场景慎用）


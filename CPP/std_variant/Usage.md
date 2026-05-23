
> 用户希望详细了解`std::variant`的用法，并给出完整代码。可能已经了解了基本概念，现在需要更深入的实际应用。需要覆盖`std::variant`的声明、赋值、访问方法、异常处理、与`std::visit`的结合使用，以及实际应用场景的示例。
首先，用户可能需要一个简单的示例，展示如何声明和初始化`std::variant`，如何检查当前存储的类型，如何安全地获取值。例如，一个可以存储整数、浮点数和字符串的variant。
接下来，用户可能想了解如何通过`std::get`和`std::get_if`访问值，以及处理可能的异常。例如，当尝试获取错误类型时抛出异常，或者使用`get_if`检查指针是否存在。
然后，用户可能对`std::visit`的使用感兴趣，特别是结合泛型lambda和`if constexpr`来处理不同类型的值。例如，编写一个访问者，根据当前存储的类型执行不同的操作。
用户可能还想看到`std::variant`在更复杂场景中的应用，比如解析不同数据类型的配置项、实现状态机、或者构建类似JSON的数据结构。例如，用variant表示JSON中的不同值类型（null、bool、数字、字符串、数组、对象）。
需要提供错误处理的示例，比如当访问不正确的类型时捕获异常，或者使用`std::holds_alternative`进行类型检查。
另外，用户需要了解`std::monostate`的作用，用于允许variant有一个空状态。例如，当variant需要表示“无值”时，可以包含`std::monostate`作为可选项。
最后，展示如何在实际项目中使用`std::variant`，比如处理多种返回类型的函数，或者替代传统的继承多态结构。


以下是一个完整的 `std::variant` 使用示例，涵盖主要功能和使用场景：

---

### **示例 1：基础用法**
```cpp
#include <variant>
#include <string>
#include <iostream>
#include <vector>

// 定义可存储的类型：int, double, string, vector<int>
using MyVariant = std::variant<int, double, std::string, std::vector<int>>;

void basic_usage() {
    MyVariant v;

    // 存储 int
    v = 42;
    std::cout << "当前值: " << std::get<int>(v) << "\n";

    // 存储 double
    v = 3.14;
    std::cout << "当前值: " << std::get<double>(v) << "\n";

    // 存储 string
    v = "Hello Variant";
    std::cout << "当前值: " << std::get<std::string>(v) << "\n";

    // 存储 vector
    v = std::vector{1, 2, 3};
    auto& vec = std::get<std::vector<int>>(v);
    vec.push_back(4);
    std::cout << "Vector大小: " << vec.size() << "\n";

    // 错误访问示例
    try {
        std::cout << std::get<int>(v) << "\n";
    } catch (const std::bad_variant_access& e) {
        std::cerr << "访问错误: " << e.what() << "\n";
    }
}
```

---

### **示例 2：类型检查与安全访问**
```cpp
void type_checking() {
    MyVariant v = 3.14;

    // 检查当前类型索引
    std::cout << "类型索引: " << v.index() << "\n";  // 输出 1

    // 类型检查
    if (std::holds_alternative<double>(v)) {
        std::cout << "当前存储的是double\n";
    }

    // 安全访问（返回指针）
    if (auto* p = std::get_if<int>(&v)) {
        std::cout << "Int值: " << *p << "\n";
    } else {
        std::cout << "当前不是int类型\n";
    }
}
```

---

### **示例 3：使用 std::visit**
```cpp
// 访问者对象
struct Visitor {
    void operator()(int val) const {
        std::cout << "收到int: " << val << "\n";
    }
    
    void operator()(double val) const {
        std::cout << "收到double: " << val << "\n";
    }
    
    void operator()(const std::string& val) const {
        std::cout << "收到string: " << val << "\n";
    }
    
    void operator()(const std::vector<int>& val) const {
        std::cout << "收到vector，大小: " << val.size() << "\n";
    }
};

void visit_demo() {
    MyVariant v1 = 42;
    MyVariant v2 = "Test";
    MyVariant v3 = std::vector{9, 8, 7};

    Visitor vis;
    std::visit(vis, v1);  // 输出: 收到int: 42
    std::visit(vis, v2);  // 输出: 收到string: Test
    std::visit(vis, v3);  // 输出: 收到vector，大小: 3
}
```

---

### **示例 4：使用 lambda + if constexpr**
```cpp
void lambda_visit() {
    MyVariant v = 3.14;

    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "整数: " << arg << "\n";
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "双精度: " << arg*2 << "\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "字符串长度: " << arg.size() << "\n";
        } else {
            std::cout << "未知类型\n";
        }
    }, v);
}
```

---

### **示例 5：JSON 数据类型模拟**
```cpp
#include <map>
#include <vector>

struct JsonNull {};
using JsonValue = std::variant<
    JsonNull,
    bool,
    int,
    double,
    std::string,
    std::vector<JsonValue>,
    std::map<std::string, JsonValue>
>;

void print_json(const JsonValue& value) {
    std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        
        if constexpr (std::is_same_v<T, JsonNull>) {
            std::cout << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (v ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int>) {
            std::cout << v;
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << v;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << '"' << v << '"';
        } else if constexpr (std::is_same_v<T, std::vector<JsonValue>>) {
            std::cout << "[ ";
            for (const auto& item : v) {
                print_json(item);
                std::cout << " ";
            }
            std::cout << "]";
        } else if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) {
            std::cout << "{ ";
            for (const auto& [key, val] : v) {
                std::cout << '"' << key << "\": ";
                print_json(val);
                std::cout << " ";
            }
            std::cout << "}";
        }
    }, value);
}

void json_example() {
    JsonValue data = std::map<std::string, JsonValue>{
        {"name", "Alice"},
        {"age", 30},
        {"scores", std::vector<JsonValue>{95, 88.5, true}},
        {"meta", JsonNull{}}
    };

    print_json(data);
    // 输出: { "age": 30 "meta": null "name": "Alice" "scores": [ 95 88.5 true ] }
}
```

---

### **示例 6：状态机实现**
```cpp
struct Idle {};
struct Connecting { std::string address; };
struct Connected { int session_id; };
struct Disconnecting {};

using ConnectionState = std::variant<Idle, Connecting, Connected, Disconnecting>;

void handle_state(const ConnectionState& state) {
    std::visit([](auto&& s) {
        using T = std::decay_t<decltype(s)>;
        
        if constexpr (std::is_same_v<T, Idle>) {
            std::cout << "处于空闲状态\n";
        } else if constexpr (std::is_same_v<T, Connecting>) {
            std::cout << "正在连接至: " << s.address << "\n";
        } else if constexpr (std::is_same_v<T, Connected>) {
            std::cout << "已连接，会话ID: " << s.session_id << "\n";
        } else {
            std::cout << "正在断开连接...\n";
        }
    }, state);
}

void state_machine_demo() {
    ConnectionState state = Idle{};
    handle_state(state);  // 处于空闲状态

    state = Connecting{"127.0.0.1"};
    handle_state(state);  // 正在连接至: 127.0.0.1

    state = Connected{1001};
    handle_state(state);  // 已连接，会话ID: 1001

    state = Disconnecting{};
    handle_state(state);  // 正在断开连接...
}
```

---

### **示例 7：使用 monostate 处理空状态**
```cpp
#include <variant>

using OptionalInt = std::variant<std::monostate, int>;

void process(const OptionalInt& value) {
    if (std::holds_alternative<int>(value)) {
        std::cout << "值: " << std::get<int>(value) << "\n";
    } else {
        std::cout << "无有效值\n";
    }
}

void monostate_demo() {
    OptionalInt v1;
    process(v1);  // 无有效值

    v1 = 42;
    process(v1);  // 值: 42
}
```

---

### **示例 8：异常处理**
```cpp
void exception_handling() {
    std::variant<int, std::string> v = "hello";

    try {
        // 尝试获取不存在的类型
        auto num = std::get<int>(v);
    } catch (const std::bad_variant_access& e) {
        std::cerr << "错误捕获: " << e.what() << "\n";
    }

    // 安全转换
    if (auto* p = std::get_if<int>(&v)) {
        // 不会执行
    } else {
        std::cout << "安全转换失败\n";
    }
}
```

---

### **编译与运行**
使用 C++17 或更高标准编译：
```bash
g++ -std=c++17 variant_demo.cpp -o variant_demo
./variant_demo
```

---

### **关键点总结**
1. **类型安全**：确保操作的类型在声明列表中
2. **访问方式**：
    - `std::get<T>`：直接访问（可能抛出异常）
    - `std::get_if<T>`：安全指针检查
    - `std::visit`：类型安全的访问模式
3. **内存管理**：自动处理构造/析构
4. **应用场景**：
    - 替代传统 union
    - 状态机实现
    - 复杂数据表示（如JSON）
    - 多态返回值

这些示例展示了 `std::variant` 在不同场景下的灵活应用，实际使用时需要根据具体需求选择最合适的访问方式和错误处理策略。
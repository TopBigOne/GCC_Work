

`std::variant` 是 C++17 引入的类型安全联合体，用于替代传统的 C 风格 `union`。以下是详细说明：

---

### **一、核心特性**
1. **类型安全**：保证存储值类型在声明时指定的类型列表中
2. **值语义**  ：   自动管理生命周期（构造/析构）
3. **空间效率**：分配足够容纳最大类型的内存
4. **访问安全**：通过类型检查防止非法访问

---

### **二、基本用法**

#### 1. 声明与初始化
```cpp
#include <variant>
#include <string>

// 可存储 int, double, string 的变体
std::variant<int, double, std::string> v;

v = 42;          // 存储 int
v = 3.14;        // 改为存储 double
v = "hello";     // 改为存储 string（构造临时对象）
```

#### 2. 访问值
```cpp
// 通过索引访问（从0开始）
double d = std::get<1>(v);  // 如果当前类型不符则抛出异常

// 通过类型访问
std::string s = std::get<std::string>(v);

// 安全访问（返回指针）
if (auto* p = std::get_if<int>(&v)) {
    std::cout << "Int value: " << *p;
}
```

#### 3. 类型判断
```cpp
// 检查当前存储类型
if (v.index() == 1) {  // 对应 double
    // ...
}

// 检查是否持有特定类型
if (std::holds_alternative<std::string>(v)) {
    // ...
}
```

---

### **三、核心功能示例**

#### 示例 1：错误处理
```cpp
std::variant<int, std::string> parse_input(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return s;  // 返回原始字符串
    }
}

void handle_result(const auto& result) {
    if (auto* val = std::get_if<int>(&result)) {
        std::cout << "Parsed int: " << *val;
    } else {
        std::cout << "Invalid input: " << std::get<std::string>(result);
    }
}
```

#### 示例 2：状态机
```cpp
struct Connecting {};
struct Connected { std::string address; };
struct Disconnected {};

using ConnectionState = std::variant<Connecting, Connected, Disconnected>;

void handle_state(ConnectionState state) {
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Connecting>) {
            std::cout << "Connecting...";
        } else if constexpr (std::is_same_v<T, Connected>) {
            std::cout << "Connected to " << arg.address;
        } else {
            std::cout << "Disconnected";
        }
    }, state);
}
```

---

### **四、高级用法**

#### 1. 使用 std::visit 访问
```cpp
std::variant<int, double> v = 3.14;

std::visit([](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>) {
        std::cout << "Int: " << arg;
    } else if constexpr (std::is_same_v<T, double>) {
        std::cout << "Double: " << arg;
    }
}, v);
```
* [std::decay_t](https://cn.cppreference.com/w/cpp/types/decay.html)
* [std::is_same_v](https://en.cppreference.com/w/cpp/types/is_same.html)

#### 2. 多态 lambda 简化
```cpp
auto printer = [](const auto& value) {
    std::cout << value << "\n";
};

std::variant<int, std::string> v1 = 42;
std::variant<double, char> v2 = 'A';

std::visit(printer, v1);  // 输出 42
std::visit(printer, v2);  // 输出 A
```

#### 3. 递归变体（需要包装）
```cpp
struct Node;
using NodeVariant = std::variant<int, std::vector<Node>>;

struct Node {
    NodeVariant data;
};

Node tree{
    std::vector{
        Node{42},
        Node{std::vector{Node{3.14}}}
    }
};
```

---

### **五、与传统 union 对比**
| 特性                | `std::variant`            | C union                  |
|---------------------|---------------------------|--------------------------|
| 类型安全            | ✅ 编译时检查              | ❌ 可能类型混淆           |
| 非POD类型支持       | ✅ 支持                    | ❌ 仅限POD类型            |
| 自动析构            | ✅ 自动调用析构函数        | ❌ 手动管理              |
| 访问检查            | ✅ 运行时检查              | ❌ 无检查                |
| 内存占用            | 类型标签 + 最大类型大小   | 最大类型大小             |
| C++标准             | C++17                     | C语言特性                |

---

### **六、最佳实践**
1. **优先替代 union**：除非有严格的内存限制
2. **配合 visit 使用**：确保处理所有类型
3. **限制类型数量**：建议不超过5种类型
4. **使用 monostate**：处理空状态
   ```cpp
   std::variant<std::monostate, int, string> v;  // 可空
   ```
5. **异常处理**：访问错误类型会抛出 `bad_variant_access`

---

### **七、性能特点**
1. **内存布局**：存储大小 = max(sizeof(Types...)
   ```cpp
   sizeof(variant<int, double>) == 通常16字节（8+对齐）
   ```
2. **访问开销**：类型检查 + 可能的跳转
3. **适合场景**：类型数量有限的小型对象

---

### **八、典型应用场景**
1. **配置项存储**：保存不同类型的配置值
2. **解析结果**：可能返回不同数据类型的解析器
3. **状态管理**：有限状态机的状态存储
4. **表达式树**：处理多种类型的节点
5. **替代继承**：当类型集合固定时

---

### **九、完整示例：JSON 值表示**
```cpp
struct JsonNull {};
using JsonValue = std::variant<
    JsonNull,
    bool,
    int,
    double,
    std::string,
    std::map<std::string, JsonValue>,
    std::vector<JsonValue>
>;

void print_json(const JsonValue& val) {
    std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, JsonNull>) {
            std::cout << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (v ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
            std::cout << v;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << '"' << v << '"';
        } else if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) {
            std::cout << "{ ... }";
        } else {
            std::cout << "[ ... ]";
        }
    }, val);
}
```

---

### **十、注意事项**
1. **默认构造**：使用第一个可默认构造类型
   ```cpp
   variant<std::string, int> v;  // 包含空字符串
   ```
2. **类型重复**：不允许相同类型
   ```cpp
   variant<int, int> v;  // 编译错误
   ```
3. **引用类型**：需要包装
   ```cpp
   variant<std::reference_wrapper<int>> v;
   ```

`std::variant` 为 C++ 带来了更安全的联合类型支持，在需要处理有限多种已知类型的场景下，是比继承更轻量、比 `void*` 更安全的解决方案。
# 学习 C++17 std::string_view

## 背景

`std::string_view` 是 C++17 引入的**非拥有型字符串视图**，本质是 `const char* data + size_t size`，核心价值是**零拷贝**地引用已有字符串数据，广泛用于性能敏感的接口设计。

---

## 模块划分

| # | 模块 | 核心知识点 | Demo 文件 |
|---|------|-----------|-----------|
| 1 | 基本概念与构造 | 什么是 view、与 `std::string` 的区别、构造方式 | `demo_basic.cpp` |
| 2 | 常用操作 | `substr`、`find`、`starts_with`、`ends_with`、`remove_prefix/suffix` | `demo_operations.cpp` |
| 3 | 性能对比 | 传参时 `string_view` vs `const string&` vs `const char*` 的开销 | `demo_perf.cpp` |
| 4 | 生命周期陷阱 | 悬空引用、临时对象问题、与 `string` 转换 | `demo_lifetime.cpp` |
| 5 | 实战场景 | 字符串解析、split、路径处理、协议解包 | `demo_practical.cpp` |

---

## 模块详解

### 模块 1 — 基本概念与构造

- `string_view` 本质：`const char* data` + `size_t size`，不持有内存
- 构造来源：字符串字面量、`std::string`、`char[]`、子串范围
- 重点：`data()` 不保证 null-terminated，不能直接传给 C API

```cpp
std::string_view sv1 = "hello";               // 从字面量构造
std::string str = "world";
std::string_view sv2 = str;                   // 从 std::string 构造
std::string_view sv3 = sv1.substr(1, 3);      // 零拷贝子串 -> "ell"
```

---

### 模块 2 — 常用操作

- `substr(pos, len)` 返回新的 `string_view`（零拷贝）
- `find` / `rfind` / `find_first_of` / `find_last_of`
- C++20 新增：`starts_with` / `ends_with`
- `remove_prefix(n)` / `remove_suffix(n)` 原地收缩视图

```cpp
std::string_view sv = "hello world";
sv.remove_prefix(6);          // sv -> "world"
sv.starts_with("wo");         // true (C++20)
sv.find('o');                 // 1
```

---

### 模块 3 — 性能对比

- 函数参数类型选择：`void f(std::string_view sv)` 是现代 C++ 推荐写法
- 对比 `const std::string&`：避免从字面量/`char*` 隐式构造 `string`（堆分配）
- 用简单计时感受实际差异

```cpp
// 推荐：无论传入 string、char*、字面量，均无拷贝
void process(std::string_view sv);

// 旧写法：传入 "hello" 会隐式构造临时 std::string（堆分配）
void process(const std::string& s);
```

---

### 模块 4 — 生命周期陷阱（重点）

- `string_view` 不延长所引用对象的生命周期
- 常见 Bug：将临时 `string` 赋给 `string_view` 后使用 -> 悬空引用/UB
- 与 `string` 互转：`std::string(sv)` 显式构造

```cpp
// 危险：临时 string 析构后 sv 悬空
std::string_view sv = std::string("temp");  // UB!

// 安全：确保原始数据生命周期覆盖 view
std::string s = "safe";
std::string_view sv2 = s;  // OK，s 存活期间 sv2 有效
```

---

### 模块 5 — 实战场景

- 手写 `split(string_view, char delim)` 返回 `vector<string_view>`
- 解析 `"key=value"` 键值对格式
- HTTP 请求行解析示意

```cpp
// split 示例
std::vector<std::string_view> split(std::string_view s, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0, pos = 0;
    while ((pos = s.find(delim, start)) != std::string_view::npos) {
        result.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    result.push_back(s.substr(start));
    return result;
}
```

---

## 推进方式

每个模块建议按如下步骤操作：

1. 新建对应 Demo 文件，在 `main.cpp` 中调用
2. 编写代码 -> 编译运行 -> 观察输出
3. 模块 4 中故意触发生命周期陷阱，观察崩溃或 UB 行为

## 参考

- [cppreference: std::basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

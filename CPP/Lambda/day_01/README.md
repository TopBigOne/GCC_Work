# Using std::function<> for lambda

* The auto keyword can be replaced with std::function<> template;
# Benefits of Using Lambda
![P one](doc/Benefits%20of%20Using%20Lambda.png)
----

以下是一个针对 **C++ Lambda 表达式**的渐进式学习计划，分为 **4 个阶段**，从基础到实战，帮助你系统掌握这一核心特性：
---

### **阶段 1：理解基础语法（1-2 天）**
#### **目标**
掌握 Lambda 的基本语法结构，理解其核心组成部分。

#### **学习内容**
1. **Lambda 表达式的基本形式**
   ```cpp
   [capture](parameters) -> return_type { 
       // 函数体 
   }
   ```
    - `capture`：捕获列表（控制外部变量的访问方式）
    - `parameters`：参数列表（类似普通函数）
    - `return_type`：返回类型（可省略，由编译器推导）

2. **最简单的 Lambda**
   ```cpp
   auto hello = [] { 
       std::cout << "Hello Lambda!"; 
   };
   hello(); // 输出 Hello Lambda!
   ```

3. **带参数的 Lambda**
   ```cpp
   auto add = [](int a, int b) { 
       return a + b; 
   };
   std::cout << add(3, 5); // 输出 8
   ```

---

### **阶段 2：掌握捕获机制（2-3 天）**
#### **目标**
理解 Lambda 如何通过捕获列表访问外部变量。

#### **学习内容**
1. **捕获方式**
    - **值捕获** `[x]`：复制外部变量的值
    - **引用捕获** `[&x]`：引用外部变量
    - **隐式捕获**：
        - `[=]`：以值捕获所有外部变量
        - `[&]`：以引用捕获所有外部变量
    - **混合捕获** `[x, &y]`

2. **示例：值捕获 vs 引用捕获**
   ```cpp
   int a = 10;
   auto lambda1 = [a] { return a; };  // 捕获时 a=10
   auto lambda2 = [&a] { return a; }; // 引用实时 a
   a = 20;
   std::cout << lambda1(); // 输出 10
   std::cout << lambda2(); // 输出 20
   ```

3. **mutable 关键字**  
   允许修改值捕获的变量（默认 Lambda 的 `operator()` 是 const）：
   ```cpp
   int x = 0;
   auto f = [x]() mutable { 
       x++; // 允许修改
       return x; 
   };
   ```

#### **练习**
- 写一个 Lambda，捕获外部变量并修改其值（使用 `mutable`）。
- 实现一个计数器 Lambda，每次调用时内部值自增。

---

### **阶段 3：实战应用（3-5 天）**
#### **目标**
将 Lambda 应用于实际场景，熟悉其在 STL 和多线程中的使用。

#### **学习内容**
1. **与 STL 算法结合**
   ```cpp
   std::vector<int> nums = {3, 1, 4, 1, 5};
   // 使用 Lambda 排序
   std::sort(nums.begin(), nums.end(), [](int a, int b) {
       return a > b; // 降序排列
   });
   ```

2. **作为回调函数**
   ```cpp
   void processData(const std::function<void(int)>& callback) {
       callback(42);
   }
   processData([](int result) {
       std::cout << "Result: " << result;
   });
   ```

3. **多线程中的 Lambda**
   ```cpp
   #include <thread>
   std::thread t([] {
       std::cout << "Running in a thread!";
   });
   t.join();
   ```

#### **练习**
- 用 Lambda 实现 `std::for_each` 自定义操作（如对每个元素平方）。
- 写一个线程池任务用 Lambda 封装。

---

### **阶段 4：高级特性与优化（2-3 天）**
#### **目标**
掌握泛型 Lambda、性能优化技巧和常见陷阱。

#### **学习内容**
1. **泛型 Lambda（C++14+）**
   ```cpp
   auto generic = [](auto x, auto y) { 
       return x + y; 
   };
   ```

2. **捕获 `*this`（C++17）**  
   避免悬垂引用：
   ```cpp
   class MyClass {
   public:
       void run() {
           auto lambda = [*this] { 
               // 安全捕获当前对象的副本
           };
       }
   };
   ```

3. **性能优化**
    - Lambda 通常会被编译器内联优化。
    - 避免在频繁调用的路径中捕获大型对象（按值捕获可能产生拷贝开销）。

#### **练习**
- 实现一个泛型 Lambda，支持不同类型的数据相加。
- 对比值捕获和引用捕获的性能差异（使用大对象测试）。

---

### **学习资源推荐**
1. **书籍**
    - 《Effective Modern C++》（条款 31-34 专门讲解 Lambda）
    - 《C++ Primer》第 14.8 章

2. **在线工具**
    - [Compiler Explorer](https://godbolt.org/)：实时查看 Lambda 的汇编代码
    - [CppReference Lambda 文档](https://en.cppreference.com/w/cpp/language/lambda)

3. **实战项目**
    - 用 Lambda 重构旧代码中的函数对象。
    - 实现一个基于 Lambda 的事件处理系统。

---

### **总结**
通过这个计划，你将从基础语法逐步深入到实际工程应用，最终掌握 Lambda 的高效使用。**关键点**：
- 理解捕获机制的作用域和生命周期
- 熟练与 STL 算法、多线程结合
- 注意性能与安全性（如避免悬垂引用）

遇到问题时，多写代码验证，善用调试工具观察行为。



---

# link
[C++ LAMBDA SYNTAX](https://www.youtube.com/watch?v=wd2LyLfvkVw)
### C++ 模板
C++模板是一种通用编程工具，它允许在编写代码时使用类型参数，以创建可以适用于不同类型的通用函数或类。模板是C++中的泛型编程机制，它可以提高代码的重用性和可扩展性。

以下是C++模板的一些常见用法：

1. **函数模板（Function Templates）**：

函数模板允许你定义一个函数，其中某些类型可以作为参数进行通用化。例如：

```cpp
template<typename T>
T add(T a, T b) {
    return a + b;
}

int result1 = add(3, 5);           // 使用int类型
double result2 = add(3.14, 2.71);  // 使用double类型
```

在上面的例子中，`add`函数模板使用了类型参数`T`，可以用于整数、浮点数等不同类型的参数。

2. **类模板（Class Templates）**：

类模板允许你定义一个通用的类，其中某些成员或方法可以适应不同类型。例如：

```cpp
template<typename T>
class Stack {
private:
    T* elements;
    int size;
public:
    // 构造函数
    Stack() {
        elements = new T[100];
        size = 0;
    }

    // 其他成员函数
    void push(T item) {
        elements[size++] = item;
    }

    T pop() {
        return elements[--size];
    }
};

// 使用类模板
Stack<int> intStack;
intStack.push(10);
intStack.push(20);
int value = intStack.pop();  // 弹出最后一个元素
```

在上面的例子中，`Stack`类模板使用了类型参数`T`，可以用于创建适用于不同类型的堆栈对象。

3. **模板特化（Template Specialization）**：

模板特化允许你为特定的类型提供定制的实现。例如：

```cpp
template<typename T>
void print(T value) {
    std::cout << value << std::endl;
}

// 特化版本用于字符串类型
template<>
void print<std::string>(std::string value) {
    std::cout << "String: " << value << std::endl;
}

print(10);                    // 输出：10
print(3.14);                  // 输出：3.14
print(std::string("Hello"));  // 输出：String: Hello
```

在上面的例子中，`print`函数模板有一个通用版本，还有一个特化版本用于处理`std::string`类型。

这只是C++模板的一小部分用法，模板还支持更复杂的特性，如模板参数的默认值、模板元编程等。模板的使用可以提供灵活性和代码重用性，但也需要注意模板的编译和实例化规则，以及可能导致代码膨胀的情况。
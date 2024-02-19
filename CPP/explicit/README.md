# explicit 是 C++ 中用来修饰类构造函数的关键字。它的作用是禁止编译器进行隐式类型转换，确保只有明确的构造调用才会发生。
# explicit 关键字，解决了什么问题？
1. 防止隐式类型转换： 当构造函数只有一个参数时，它可以被用于隐式类型转换。这可能导致意外的行为和代码错误。使用 explicit 关键字可以禁止这种隐式转换，强制要求使用显式构造。
```C++
class MyClass {
public:
    explicit MyClass(int value) : data(value) {}
private:
    int data;
};

 ```
> 如果没有 explicit，则可以发生隐式转换：MyClass obj = 42;
2. 避免二义性： 在某些情况下，如果没有 explicit，可能会导致构造函数的二义性，使编译器无法确定要使用哪个构造函数。
```C++
class MyClass {
public:
    explicit MyClass(int value) : data(value) {}
    MyClass(double value) : data(static_cast<int>(value)) {}
private:
    int data;
};

 ```
> 如果没有 explicit，则 MyClass obj = 42.0; 可能会变得模糊，因为有两个构造函数都可以接受 42.0。

### 说白了
* 使用 explicit 可以提高代码的明确性和安全性，防止一些潜在的错误。




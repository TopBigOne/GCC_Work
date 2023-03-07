### [noexcept 运算符](https://veitchkyrie.github.io/2020/02/24/C++-noexcept-%E8%BF%90%E7%AE%97%E7%AC%A6/)
#### noexcept 是一个运算符，有以下两种形式：
```c++
noexcept
noexcept(expression)

```
* 第一种形式等同于 noexcept(true)。当 noexcept 用于函数声明时，指定了函数是否会抛出异常。noexcept(true) 表示被修饰的函数不抛出异常，noexcept(false) 表示被修饰的函数会抛出异常。

* noexcept 操作符的用法如下：
```c++
#include<iostream> 
void BlockThrow() noexcept {
  throw 1;
}

int main(void) {
  try {
    BlockThrow();
  } catch(...) {
    std::cout << "BlockThrow Found throw" << std::endl;
  }
}

==== 输出：
terminate called after throwing an instance of 'int'
Aborted

```
* **noexcept 操作符的作用是阻止异常的传播**。从上面代码可以看出，当使用 noexcept 修饰的函数内部抛出异常时，会立即调用 std::terminate 退出进程，程序的执行并没有进入到 main 函数中的 catch 块中，这样就阻止了异常向上层的传播。

#### 带参数的使用方法：
```c++
void* operator new(std::size) noexcept(false);

```
* 这个语句表示 new 操作符是可以抛出异常的。

* 需要注意的一点是，noexcept(expression) 中的expresson 的值是在编译期间进行检查的，
* 在计算expression 后，编译器最终会将expression 的结果转换为 ture 或false，之后再传递给 noexcept()，
* 因而，可以认为，noexcept(expression) 
* 最终是被转换为 noexcept(true) 或 noexcept(false) 之后才作用于函数声明的。
* 由于expression 值是在编译期间检查的，这一值必须是在编译期间可以确定的，
  * 例如 const 类型、定义的宏、函数调用等。编译器内 expression 与bool值的转换规则如下：
   > 当 expression 为如下表达式时，会转换为false，否则转换为true：
   > 1. expression 是一个函数调用，且此函数声明中无 noexcept 声明
   > 2. expression 是一个 throw 表达式
* 既然 noexcept(expression) 是一个操作符，那么其必然有一个返回值，它的返回值是false 或true，
* 注意这里涉及的是返回值，要与上述expression 转换为bool 值相区分。
* 当其修饰函数声明时，我们通常看不到其返回值，其单独使用时，我们能看到返回值。
* 若 noexcept(expression) 返回true，则表示当此表达式修饰函数时，
* 不允许抛出异常，否则允许抛出异常。因而若某个 noexcept(expression) 返回true，
* 那么当此表达式修饰函数时，相当于使用了 noexcept(true)，否则相当于使用了 noexcept(false)。

* 可以用以下程序验证上述转换：
```c++
#include <iostream> 
void may_throw();
void may_throw1() noexcept(false);
void no_throw() noexcept;
auto lmay_throw = []{};
auto lmay_throw1 = []() noexcept(false) {};
auto lno_throw = []() noexcept {};

int main() {
 std::cout << std::boolalpha
           << "Is may_throw() noexcept? " << noexcept(may_throw()) << '\n'
           << "Is may_throw1() noexcept? " << noexcept(may_throw1()) << '\n'
           << "Is no_throw() noexcept? " << noexcept(no_throw()) << '\n'
           << "Is lmay_throw() noexcept? " << noexcept(lmay_throw()) << '\n'
           << "Is lmay_throw1() noexcept? " << noexcept(lmay_throw1()) << '\n'
           << "Is lno_throw() noexcept? " << noexcept(lno_throw()) << '\n';
}

```
* 需要注意的是，C++11 出于安全考虑，如果一个类的析构函数没有显式的声明 noexcept(expression)，那么这一析构函数默认是为 noexcept(true) 的。


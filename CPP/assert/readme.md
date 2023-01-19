### 断言
* 表达式：
```c++
 void assert( int expression );
 ```
> assert()接受一个表达式expression作为参数，如果表达式值为真，继续往下执行程序，如果表达式值为假，assert()会调用abort()函数终止程序的执行，并提示失败信息。
* 取消断言
> #define NDEBUG
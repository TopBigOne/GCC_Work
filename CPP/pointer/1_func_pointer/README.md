# 函数指针

* 函数也是有地址的。
* 在程序中定义一个函数，则编译时系统就会为这段代码分配一段存储空间，这段存储空间的首地址称为函数的地址，且函数名代表的就是这个地址。既然是地址就可以使用一个指针变量来存储这个地址，这个指针变量就就叫做函数指针。
### 格式：
* 函数返回值类型 (* 指针变量名) (函数参数列表);
* int (*p)(int,int)
> (*指针变量名)这个括号不能省略，如果写成了int *p(int,int)，那就不是一个函数指针了，而是一个返回值类型为一个int型指针的函数p。
### Q:(*指针变量名)这个括号不能省略，如果写成了int *p(int,int)，那就不是一个函数指针了，而是一个返回值类型为一个int型指针的函数p。

* 1: 首先看变量名前面有没有“ ”，如果有“ ”说明是指针变量；

* 2: 看变量名的后面有没有带有形参类型的圆括号，如果有就是指向函数的指针变量，即函数指针，
* 3: 如果没有形参类型的圆括号,就是指向变量的指针变量。
### 如何使用函数指针
```c++
int max(int x, int y) {
    return x >= y ? x : y;
}
int min(int x, int y) {
    return x >= y ? y : x;
}

void extendsTest::mainTest()
{   
    int a = 10, b = 20;
    int (*p)(int, int);
    p = max;
    cout << p(a, b) << endl;
    p = min;
    cout << p(a, b) << endl;
};
// 运行结果：
// 20
// 10
```

* 2: 作为参数进行函数回调
> 大部分情况下在代码中函数指针是被用来函数回调使用，也就是作为参数传递
```c++
int max(int x, int y) {
    return x >= y ? x : y;
}
int min(int x, int y) {
    return x >= y ? y : x;
}
//指针形式
void func(int& a,int& b,int (*p)(int,int)) {
    cout << p(a, b) << endl;
}
//引用形式
void func1(int& a, int& b, int (&p)(int, int)) {
    cout << p(a, b) << endl;
}

void extendsTest::mainTest()
{   
    int a = 10, b = 20;
    func(a, b, max);
    func(a, b, min);
};

//运行结果：
//20
//10

```
## 回调函数
### 参考
* https://mp.weixin.qq.com/s/Q3MuhUrfglXi6xHTjYTEKg
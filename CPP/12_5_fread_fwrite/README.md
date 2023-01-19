### fread[d和fwrite的用法详解（以数据块的形式读写文件）](http://c.biancheng.net/view/2071.html)

* fread() 函数用来从指定文件中读取块数据。所谓块数据，也就是若干个字节的数据，可以是一个字符，可以是一个字符串，可以是多行数据，并没有什么限制。fread() 的原型为：
```C
size_t fread ( void *ptr, size_t size, size_t count, FILE *fp );
```
fwrite() 函数用来向文件中写入块数据，它的原型为：
```C
size_t fwrite ( void * ptr, size_t size, size_t count, FILE *fp );
```
> 对参数的说明：
ptr 为内存区块的指针，它可以是数组、变量、结构体等。fread() 中的 ptr 用来存放读取到的数据，fwrite() 中的 ptr 用来存放要写入的数据。
size：表示每个数据块的字节数。
count：表示要读写的数据块的块数。
fp：表示文件指针。
理论上，每次读写 size*count 个字节的数据。

size_t 是在 stdio.h 和 stdlib.h 头文件中使用 typedef 定义的数据类型，表示无符号整数，也即非负数，常用来表示数量。

> 返回值：返回成功读写的块数，也即 count。如果返回值小于 count：
对于 fwrite() 来说，肯定发生了写入错误，可以用 ferror() 函数检测。
对于 fread() 来说，可能读到了文件末尾，可能发生了错误，可以用 ferror() 或 feof() 检测。
```C++
#include <unistd.h>
int dup2(int oldfd, int newfd);

```
* 这个函数的作用是将文件描述符 oldfd 复制到文件描述符 newfd，并关闭 newfd 引用的文件描述符（如果 newfd 已经打开）。
* 简而言之，dup2 用于重定向文件描述符。 常见的用法是在进行输入/输出重定向时，将标准输入或标准输出重定向到文件或管道
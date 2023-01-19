* 把参数 char 指定的字符（一个无符号字符）写入到标准输出 stdout 中。


```c
int putchar(int char)
```


```c
#include <stdio.h>

int main ()
{
   char ch;

   for(ch = 'A' ; ch <= 'Z' ; ch++) {
      putchar(ch);
   }
   
   return(0);
}

```
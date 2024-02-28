### 安全的strlcpy和 strlcat 字符串操作函数


* 过C语言的肯定都知道strcpy和strcat，但是这两个函数有个致命的缺陷，它们不检查dst是否有足够的空间，
* 如果src足够长必然会导致缓冲区溢出，于是有就了改进版strncpy和strncat，
* 这两个函数在一定程度上解决了安全问题，但是很多程序员都不愿使用它们，


```C++
size_t strlcat(char* dest, const char* src, size_t size);
```
* dest：目标字符串，要将源字符串追加到其中。
* src：源字符串，要追加到目标字符串末尾的字符串。
* size：目标字符串的最大容量大小。
-- 
### strlcat 函数会将源字符串 src 追加到目标字符串 dest 的末尾，但不会超出 size 指定的最大容量大小。它会确保目标字符串以 null 结尾，即使截断了源字符串或达到了最大容量。


---
# 直白一点，就是字符串拼接
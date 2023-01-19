# 书本《C Primer Plus》
# 第13 章
# 13.5：把文件附加到另一个文件夹的末尾 
# P 370；
------

### api 解读
---
* getchar()
> 从标准输入 stdin 获取一个字符（一个无符号字符）。这等同于 getc 带有 stdin 作为参数。 
* int putchar(int char)
>把参数 char 指定的字符（一个无符号字符）写入到标准输出 stdout 中。
* fgets();
> 获取 一行字符串
* fputs
> 写字符串
* setvbuf()
> 设置stream的缓冲类型，目前，有三种：IOFBF,IOLBF,IONBF;

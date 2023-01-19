### 随机访问 fseek() ftell();
### feek()


* 有个feek() 函数，便可以将文件看做是数组，在fopen() 打开的文件中直接移动到任意字节处；
### ### feek() ftell()的问题；
* 文件大小限制在 **long** 类型能表示的范围内，
* 所以，fgetpos() ,fsetpos() ,出现了，这两个函数使用long 类型表示位置，而是使用 **f_pos_t**;



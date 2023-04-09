### C 库函数 - fseek() **移动文件流**的读写位置
* 设置流 stream 的文件位置为给定的偏移 offset，参数 offset 意味着从给定的 whence 位置查找的字节数。
### 参数说明
* stream ： 指向FILE 对象的指针，该FILE 对象标识了流
* offset : 相对whence的偏移量，以字节为单位
* whence ： 开始添加偏移offset 的位置，有三个常量
  * seek_set : 文件的开头
  * seek_cur : 文件指针的当前位置
  * seek_end : 文件的末尾
### 备注：
 * fseek()不像lseek()会返回读写位置, 因此必须使用ftell()来取得目前读写的位置.
### 参考
* https://blog.51cto.com/u_12261659/1908526
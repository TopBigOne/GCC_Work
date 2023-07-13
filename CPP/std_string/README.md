## C++ 标准库 string 常用的函数
### size()：返回字符串的长度。
### length()：与 size() 功能相同，返回字符串的长度。
### empty()：如果字符串为空则返回 true，否则返回 false。
### clear()：清空字符串。
### substr(pos, len)：返回从 pos 开始长度为 len 的子字符串。
### append(str)：在字符串的末尾添加 str。
### insert(pos, str)：在 pos 处插入字符串 str。
### replace(pos, len, str)：用字符串 str 替换从 pos 开始长度为 len 的子字符串。
### find(str)：查找字符串 str 在当前字符串中的位置，如果找到则返回字符串的起始位置，否则返回 npos。
### rfind(str)：与 find() 功能相同，从字符串的末尾开始查找。
### compare(str)：与字符串 str 比较，如果相同则返回 0，如果当前字符串小于 str 则返回负数，否则返回正数。
### c_str()：返回一个指向字符串首字符的指针。
### at(index)：返回字符串中索引为 index 的字符。
### operator[]：重载了 [] 运算符，可以通过下标访问字符串中的字符。
### getline(input_stream, str)：从输入流中读取一行字符串并存储到 str 中。



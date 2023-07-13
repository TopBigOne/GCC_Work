//
// Created by dev on 2023/7/13.
//

#ifndef STD_STRING_USESTRING_H
#define STD_STRING_USESTRING_H


#include <cstdio>
#include <string>
#include <iostream>

class UseString {
public:
    UseString();

    ~UseString();

    /**
     * 返回字符串的长度。
     * @return
     */
    size_t my_size(std::string str_src);


    /**
     * 与 size() 功能相同，返回字符串的长度。
     * @return
     */
    size_t my_length(std::string str_src);

    /**
     * ：如果字符串为空则返回 true，否则返回 false。
     * @return
     */
    bool my_empty(std::string str_src);

    /**
     * ：清空字符串。
     */
    void my_clear(std::string str_src);

    /**
     * 返回从 pos 开始长度为 len 的子字符串，返回类型为 string，
     *
     * @param pos  第一个参数为子字符串的起始位置
     * @param len   第二个参数为子字符串的长度。
     * @return
     */
    std::string my_substr(std::string str_src, size_t pos, size_t len);

    /**
     * ：在字符串的末尾添加 str
     * @param s
     */
    std::string &my_append(std::string str_src, const std::string &s);

    /**
     * ：在 pos 处插入字符串 str。
     * @param s
     */
    std::string &my_insert(std::string str_src, size_t pos, const std::string &str);

    /**
     *用字符串 str 替换从 pos 开始长度为 len 的子字符串，无返回值，
     *
     * @param pos  子字符串的起始位置
     * @param len 为子字符串的长度
     * @param str  替换的字符串
     */
    std::string &my_replace(std::string str_src, int pos, int len,  const std::string &str);

    /**
     * 查找字符串 str 在当前字符串中的位置，
     * @param str 如果找到则返回字符串的起始位置，否则返回 npos。
     */
    size_t my_find(std::string str_src, std::string str);

    /**
     * 与 find() 功能相同，从字符串的末尾开始查找。
     * @param str
     */
    size_t my_rfind(std::string str_src, std::string str);

    /**
     *与字符串 str 比较，
     * @param str 如果相同则返回 0，如果当前字符串小于 str 则返回负数，否则返回正数。
     */
    int my_compare(std::string str_src, std::string str);

    /**
     * 返回一个指向字符串首字符的指针，返回类型为 const char*，
     */
    const char *my_c_str(std::string str_src);

    /**
     * ：返回字符串中索引为 index 的字符。
     * @param index
     */
    char &my_at(std::string str_src, int index);
    //  void  operator[]：重载了 [] 运算符，可以通过下标访问字符串中的字符。
    //   void  getline(input_stream,  std::string str)：从输入流中读取一行字符串并存储到 str 中。
    //  istream& getline(istream& input_stream, string& str, char delim = '\n');


public:
    std::string raw_str;
};


#endif //STD_STRING_USESTRING_H

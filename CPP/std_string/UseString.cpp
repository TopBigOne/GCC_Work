//
// Created by dev on 2023/7/13.
//

#include "UseString.h"


UseString::UseString() {
    // raw_str = "qwertyuiop";
    std::wcerr << "invoke 构造函数" << std::endl;
}

UseString::~UseString() {
    std::wcerr << "invoke 析构函数" << std::endl;
}

size_t UseString::my_size(std::string str) {
    return str.size();
}

size_t UseString::my_length(std::string str) {
    return str.length();
}

bool UseString::my_empty(std::string str) {
    return str.empty();
}

void UseString::my_clear(std::string str) {
    return str.clear();

}

std::string UseString::my_substr(std::string str, size_t pos, size_t len) {
    return str.substr(pos, len);
}

std::string &UseString::my_append(std::string str_src, const std::string &s) {
    //return str_src.append("我是后来添加的");
    return str_src.append(s);

}

std::string &UseString::my_insert(std::string str_src, size_t pos, const std::string &str) {
    //return raw_str.insert(3, "我是后来insert的");
    return str_src.insert(pos, str);
}

std::string &UseString::my_replace(std::string str_src, int pos, int len,const std::string &str) {
    //return str_src.replace(pos, len, "俺是replace添加的");
    return str_src.replace(pos, len, str);
}

size_t UseString::my_find(std::string str_src, const std::string str_dst) {
    // return str_src.find( "op");
    return str_src.find(str_dst);
}

size_t UseString::my_rfind(std::string str_src, const std::string str_dst) {
    return str_src.find(str_dst);
}

int UseString::my_compare(std::string str_src,  const std::string str_dst) {
    return str_src.compare(str_dst);
}

const char *UseString::my_c_str(std::string str_src) {
    return str_src.c_str();
}

char &UseString::my_at(std::string str_src, int index) {
    return str_src.at(index);
}


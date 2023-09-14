//
// Created by dev on 2023/9/15.
//

#ifndef TEMPLATE_TEMPLATESPECIALIZATION_H
#define TEMPLATE_TEMPLATESPECIALIZATION_H

#include <iostream>

/**
 * 模板特化允许你为特定的类型提供定制的实现
 */
class TemplateSpecialization {
public:
    // T : 没有使用之前，我也不知道是什么类型
    template<class T>
    void print(T value) {
        std::cout << value << std::endl;
    }

    // 特定化版本用于字符串类型
    template<>
    void print<std::string>(std::string value) {
        std::cout << "String is : " << value << std::endl;
    }

};


#endif //TEMPLATE_TEMPLATESPECIALIZATION_H

//
// Created by DEV on 2026/3/9.
//

#ifndef STD_VARIANT_VARIANTDEMO_H
#define STD_VARIANT_VARIANTDEMO_H


#include <variant>
#include <string>
#include <iostream>
#include <vector>

struct Visitor {
    void operator()(int val) {
        std::cout << "收到int :" << val << std::endl;
    }

    void operator()(double val) {
        std::cout << "收到double :" << val << std::endl;
    }

    void operator()(std::string val) {
        std::cout << "收到string :" << val << std::endl;
    }

    void operator()(std::vector<int> val) {
        std::cout << "收到vector ,size :" << val.size() << std::endl;
    }
};

class VariantDemo {
public:
    /**
     * 基础用法
     */
    void baseUsage();

    /**
     * 类型检查和安全访问
     */
    void typeChecking();

    /**
    * 使用std::visit
    */
    void withVisit();

    /**
    * 使用lambda
    */
    void withLambda();

};


#endif //STD_VARIANT_VARIANTDEMO_H

//
// Created by DEV on 2026/3/9.
//

#ifndef STD_OPTIONAL_RESOURCEHOLDER_H
#define STD_OPTIONAL_RESOURCEHOLDER_H


#include <optional>
#include "ExpensiveResource.h"
class ResourceHolder {
public:
    std::optional<ExpensiveResource> resource_;

public:
    void lazy_init() {
        if (!resource_) {
            //延迟初始化
            resource_.emplace();
        }
    }
    void use_resource() {
        // case 1:
        if (resource_) {
            resource_->use();
            return;
        }
        // case 2:
        std::cout << "Resource not initialized\n";

    }
};


#endif //STD_OPTIONAL_RESOURCEHOLDER_H
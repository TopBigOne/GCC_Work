//
// Created by DEV on 2026/3/8.
//

#include "OptionalDemo.h"
#include "ExpensiveResource.h"
#include "ResourceHolder.h"
#include <sstream>
#include <algorithm>

std::optional<int> safeDivide(int a, int b) {
    if (b == 0) { return std::nullopt; }
    return a / b;
}

void OptionalDemo::baseAbility() {
    optional<int> optInt;
    optional<string> optString = "Hello Optional<T>";
    // 检查是否有值
    if (optString.has_value()) {
        cout << optString.value() << endl;
    }

    // 安全除法
    auto result1 = safeDivide(10, 2);
    auto result2 = safeDivide(5, 0);
    cout << result1.value_or(-1) << endl;
    cout << result2.value_or(-1) << endl;
    try {
        //抛出 bad_optional_access
        result2.value();
    } catch (const std::bad_optional_access &e) {
        cerr << "bad_optional_access :" << e.what() << endl;
    }
}

template<typename T>
std::optional<size_t> findIndex(const std::vector<T> &vec, const T &target) {
    auto it = std::find(vec.begin(), vec.end(), target);
    //  检查是否找到（it != vec.end()）
    if (it != vec.end()) {
        // 如果找到，使用 std::distance 计算索引位置并返回
        return std::distance(vec.begin(), it);
    }
    return std::nullopt;
}

/***
 * 查找元素
 */
void OptionalDemo::findElement() {
    std::vector<int> numbers{1, 3, 5, 7, 9};
    if (auto index = findIndex(numbers, 5)) {
        cout << "Found at index : " << *index << endl;
    } else {
        cout << "Not found" << endl;
    }
    if (auto index = findIndex(numbers, 4)) {
        // Note :不会执行
    } else {
        cout << "4 Not found" << endl;
    }
}


void OptionalDemo::delayInit() {
    cout << "       注意：emplace()的用法" << endl;
    ResourceHolder holder;
    holder.use_resource();

    holder.lazy_init();
    holder.use_resource();
}

std::optional<Config> OptionalDemo::parseConfig(const std::string &input) {
    std::istringstream iss(input);
    Config config;
    std::string key;
    while (iss >> key) {
        if (key == "brightness:") {
            int value;
            if (iss >> value) {
                config.brightness = value;
            }
        } else if (key == "timeout:") {
            double value;
            if (iss >> value) {
                config.timeout = value;
            }
        } else if (key == "theme:") {
            std::string value;
            if (iss >> value) {
                config.theme = value;
            }
        }
    }
    //至少有一个配置，才算生效
    if (config.brightness || config.timeout || config.theme) {
        return config;
    }
    return std::nullopt;
}

void OptionalDemo::accessData() {
    Company company{
        {
            {"IT", Person{"Alice", "alice@company.com"}},
            {"HR", Person{"Bob", std::nullopt}},
            {"Executive", Person{"Carol", "ceo@company.com"}}
        }
    };

    if (auto email = findCEOEmail(company)) {
        std::cout << "CEO email: " << *email << "\n";
    } else {
        std::cout << "CEO email not found\n";
    }
}

std::optional<std::string> findCEOEmail(const Company &company) {
    for (const auto &department: company.departments) {
        if (department.name == "Executive") {
            if (department.manager) {
                if (department.manager->email) {
                    return department.manager->email;
                }
            }
        }
    }

    return std::nullopt;
}

/**
 *
* @param nums  std::optional<int> example1() {
      int value = 42;
      return value;        // 隐式转换：int → std::optional<int>
  }

  std::optional<int> example2() {
      int value = 42;
      return std::optional<int>(value);  // 显式构造
  }

 * @return
 */
std::optional<int> firstSeven(const std::vector<int> &nums) {
    auto it = std::find_if(
        nums.begin(),
        nums.end(),
        [](int n) { return n % 2 == 0; });
    if (it != nums.end()) {
        // 取值：  // 隐式转换：int → std::optional<int>
        // 编译器自动转换 int → std::optional<int>
        return *it;
    }
    return std::nullopt;
}


void OptionalDemo::withSTL() {
    std::vector<int> data{1, 3, 5, 7, 8, 9};
    if (auto num = firstSeven(data)) {
        std::cout << "First event number : " << *num << std::endl;
    } else {
        std::cout << "First event number not found" << std::endl;
    }

    std::vector<int> oldOnly{1, 3, 5, 7, 9};
    std::cout << "First even in odd set: " <<
            firstSeven(oldOnly).value_or(-1) << std::endl;
}

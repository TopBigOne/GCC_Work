//
// Created by DEV on 2026/3/2.
//

#ifndef STD_FUNCTION_STRATEGYPATTERNSORT_H
#define STD_FUNCTION_STRATEGYPATTERNSORT_H
#include <functional>
#include <iostream>
#include <ostream>
#include <vector>

/**
 * 用策略模式显示实现排序(实现比较器)
 */
class StrategyPatternSort {
private:
    std::function<bool(int, int)> compartor;

public:
    void setCompartor(std::function<bool(int, int)> comp) {
        compartor = comp;
    }

    void sotr(std::vector<int> &data) {
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = i + 1; j < data.size(); ++i) {
                if (compartor(data[i], data[j])) {
                    std::swap(data[i], data[j]);
                }
            }
        }
    }

    void test() {
        StrategyPatternSort sort;
        std::vector<int> numbers = {5, 2, 9, 1, 5};
        // 升序
        sort.setCompartor([](int a, int b) {
            return a > b;
        });

        for (int number: numbers) {
            std::cout << number;
        }
        std::cout << std::endl;

        // 降序
        sort.setCompartor([](int a, int b) {
            return a <b;
        });

        for (int number: numbers) {
            std::cout << number;
        }
        std::cout << std::endl;
    }
};


#endif //STD_FUNCTION_STRATEGYPATTERNSORT_H

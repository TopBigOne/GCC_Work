//
// Created by DEV on 2025/8/1.
//

#include "TestArray.h"

#include <array>
#include <iostream>
#include <map>


/**
 * case 1:创建
 */
void testBase() {
    std::array<int, 10> arr = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int value: arr) {
        std::cout << value << std::endl;
    }
}

/**
 * case 2:使用迭代器遍历
 */
void testIteration() {
    std::array<std::string, 3> names = {"jar", "sw-1", "xiao_ya"};
    for (auto it = names.begin(); it != names.end(); ++it) {
        std::cout << *it << std::endl;
    }
}

/**
 * case 3: 使用stl 排序
 */
void testSTLSort() {
    std::array<int, 10> arr = {1, 23, 3, 47, 5, 2, 7, 8, 977, 10};
    std::sort(arr.begin(), arr.end());
    for (int value: arr) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}


/**
 * case 4: 边界检查，
 */
void testAt() {
    std::array<int, 3> arr = {10, 20, 40};
    try {
        std::cout << arr.at(2) << std::endl;
        std::cout << arr.at(6) << std::endl;
    } catch (const std::out_of_range &e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}


/**
 * case 5: 整体赋值和copy，
 * 说白了，可以作用= 直接赋值，类似java
 */
void testCopy() {
    const std::array<int, 3> arr = {10, 20, 40};
    const std::array<int, 3> arr2 = arr;
    for (int value: arr2) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}


/**
 * case 6: 用作 STL 容器的 key
 */
void testKey() {
    std::map<std::array<int, 2>, std::string> points;
    points[{6, 2}] = "A";
    points[{9, 3}] = "B";
    std::cout << points[{6, 2}] << std::endl;
}


/**
 * case 7: 填充数组：fill 方法
 */
void testFill() {
    std::array<int, 5> arr{};
    arr.fill(8);
    for (int value: arr) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

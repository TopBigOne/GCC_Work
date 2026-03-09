//
// Created by DEV on 2026/3/8.
//

#ifndef STD_OPTIONAL_OPTIONALDEMO_H
#define STD_OPTIONAL_OPTIONALDEMO_H


#include <optional>
#include <string>
#include <iostream>
using namespace std;


struct Config {
    std::optional<int> brightness;
    std::optional<double> timeout;
    std::optional<std::string> theme;
};

struct Person {
    std::string name;
    std::optional<std::string > email;
};


struct Department {
    std::string name;
    std::optional<Person> manager;
};
struct Company {
    std::vector<Department> departments;
};

std::optional<std::string> findCEOEmail(const Company &company);

std::optional<int> firstSeven(const std::vector<int> &nums);

class OptionalDemo {
public:
    /**
     * 基础用法
     */
    void baseAbility();

    /**
     *  查找元素
     */
    void findElement();

    /**
    *  延迟初始化
    */
    void delayInit();struct

    /**
   *  解析配置
   */
    std::optional<Config> parseConfig(const std::string &input);

    /**
 *  安全访问嵌套数据
 */
    void accessData();

    /**
*  与 STL 算法结合
*/
    void withSTL();
};


#endif //STD_OPTIONAL_OPTIONALDEMO_H

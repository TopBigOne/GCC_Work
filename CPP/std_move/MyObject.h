#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class MyObject {
private:
    string name;
    vector<string> nameList;

public:
    /**
     * 默认构造函数
     * 场景：创建一个新的空对象
     * 例子：MyObject obj;
     */
    MyObject() = default;

    /**
     * 拷贝构造函数
     * 场景：用一个已有对象初始化新的对象，深拷贝
     * 例子：MyObject obj2 = obj1;
     * 触发时机：
     *   - 传值参数（如 void foo(MyObject obj)）
     *   - 返回值优化失效时
     *   - 明确用 MyObject obj2(obj1);
     * @param other 源对象
     */
    MyObject(const MyObject &other);

    /**
     * 移动构造函数
     * 场景：用一个右值对象（临时对象）初始化新的对象，资源"搬走"，原对象被置空
     * 例子：MyObject obj3 = std::move(obj1);
     * 触发时机：
     *   - 容器扩容/插入时（如 vector<MyObject> v; v.push_back(std::move(obj));）
     *   - 返回临时对象（如 return obj;）
     * @param other 源对象（右值引用）
     */
    MyObject(MyObject &&other) noexcept;

    /**
     * 拷贝赋值运算符
     * 场景：用已有对象赋值给另一个已存在对象，深拷贝
     * 例子：obj2 = obj1;
     * @param other 源对象
     * @return 当前对象的引用
     */
    MyObject &operator=(const MyObject &other);

    /**
     * 移动赋值运算符
     * 场景：用右值对象赋值给另一个已存在对象，资源"搬走"，原对象被置空
     * 例子：obj2 = std::move(obj1);
     * @param other 源对象（右值引用）
     * @return 当前对象的引用
     */
    MyObject &operator=(MyObject &&other) noexcept;

    /**
     * 析构函数
     * 场景：对象生命周期结束时自动调用，释放资源
     * 例子：obj 离开作用域时
     */
    ~MyObject() = default;

    // 业务方法
    void setName(const string &newName);

    void addNameList();

    void printObj() const;
};

#endif // MYOBJECT_H

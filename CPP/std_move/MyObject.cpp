#include "MyObject.h"

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
MyObject::MyObject(const MyObject &other) : name(other.name), nameList(other.nameList) {
    cout << "Copy constructor called" << endl;
    name = other.name;
    nameList = other.nameList;
}

/**
  * 移动构造函数
  * 场景：用一个右值对象（临时对象）初始化新的对象，资源"搬走"，原对象被置空
  * 例子：MyObject obj3 = std::move(obj1);
  * 触发时机：
  *   - 容器扩容/插入时（如 vector<MyObject> v; v.push_back(std::move(obj));）
  *   - 返回临时对象（如 return obj;）
  * @param other 源对象（右值引用）
  */
MyObject::MyObject(MyObject &&other) noexcept {
    cout << "Move constructor called" << endl;
    name = std::move(other.name);
    nameList = std::move(other.nameList);

    // Clear the source object's data (optional but good practice)
    other.name.clear();
    other.nameList.clear();
}

/**
    * 拷贝赋值运算符
    * 场景：用已有对象赋值给另一个已存在对象，深拷贝
    * 例子：obj2 = obj1;
    * @param other 源对象
    * @return 当前对象的引用
    */
MyObject &MyObject::operator=(const MyObject &other) {
    cout << "Copy assignment operator called" << endl;
    if (this != &other) {
        name = other.name;
        nameList = other.nameList;
    }
    return *this;
}

/**
   * 移动赋值运算符
   * 场景：用右值对象赋值给另一个已存在对象，资源"搬走"，原对象被置空
   * 例子：obj2 = std::move(obj1);
   * @param other 源对象（右值引用）
   * @return 当前对象的引用
   */
MyObject &MyObject::operator=(MyObject &&other) noexcept {
    cout << "Move assignment operator called" << endl;
    if (this != &other) {
        name = std::move(other.name);
        nameList = std::move(other.nameList);
        // Clear the source object's data
        other.name.clear();
        other.nameList.clear();
    }
    return *this;
}

void MyObject::setName(const string &newName) {
    name = newName;
}

void MyObject::addNameList() {
    if (!name.empty()) {
        nameList.push_back(name);
    }
}

void MyObject::printObj() const {
    std::cout << "printObj" << std::endl;
    std::cout << "   Name: ";
    if (nameList.empty()) {
        cout << "(empty)";
    } else {
        for (size_t i = 0; i < nameList.size(); ++i) {
            if (i > 0) cout << ", ";
            cout << nameList[i];
        }
    }
    cout << endl;
    cout << "---" << endl;
}

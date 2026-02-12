#ifndef MOVE_DEMO_H
#define MOVE_DEMO_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

using namespace std;

/**
 * 演示完整移动语义的类 - Rule of Five 完整实现
 */
class ResourceManager {
private:
    string name;
    vector<int> data;
    int* rawPtr;
    size_t size;

public:
    // 1. 默认构造函数
    ResourceManager();

    // 2. 带参数构造函数
    ResourceManager(const string& name, size_t dataSize);

    // 3. 析构函数
    ~ResourceManager();

    // 4. 拷贝构造函数
    ResourceManager(const ResourceManager& other);

    // 5. 移动构造函数
    ResourceManager(ResourceManager&& other) noexcept;

    // 6. 拷贝赋值运算符
    ResourceManager& operator=(const ResourceManager& other);

    // 7. 移动赋值运算符
    ResourceManager& operator=(ResourceManager&& other) noexcept;

    // 工具方法
    void printInfo() const;
    void setName(const string& newName);
    const string& getName() const;
    size_t getDataSize() const;
    bool isValid() const;

    // 静态工厂方法
    static ResourceManager createLarge(const string& name);
};

/**
 * 基础演示类 - 演示基本移动语义概念
 */
namespace BasicDemo {
    void demonstrateBasicMove();
    void demonstrateCopyVsMove();
    void demonstrateStringMove();
    void demonstrateVectorMove();
}

/**
 * 性能测试类
 */
namespace PerformanceTest {
    void compareContainerPerformance();
    void compareStringPerformance();
    void compareCustomObjectPerformance();
    void compareRealWorldScenario();
    void explainPerformanceDifference();  // 新增：性能差异分析
    void runAllPerformanceTests();
}

/**
 * 最佳实践演示
 */
namespace BestPractices {
    // 函数参数设计模式
    void setDataCopy(const string& s);
    void setDataMove(string&& s);

    template<typename T>
    void setDataPerfectForwarding(T&& s);

    // 容器操作优化
    void demonstrateContainerOptimization();

    // 智能指针转移
    void demonstrateSmartPointerMove();

    // 工厂函数最佳实践
    ResourceManager createResource(const string& name, size_t size);

    void runAllBestPractices();
}

/**
 * 常见陷阱演示
 */
namespace TrapExamples {
    void trapBasicTypeMove();           // 陷阱1：基本类型move无效
    void trapReturnValueMove();         // 陷阱2：返回值错误使用move
    void trapUseAfterMove();           // 陷阱3：移动后继续使用
    void trapTemporaryObjectMove();    // 陷阱4：临时对象多余move

    void runAllTrapExamples();
}

/**
 * 主演示控制器
 */
namespace MoveShowcase {
    void printSectionHeader(const string& title);
    void printSubHeader(const string& title);
    void pauseForUser();

    void runCompleteDemo();
}

#endif // MOVE_DEMO_H
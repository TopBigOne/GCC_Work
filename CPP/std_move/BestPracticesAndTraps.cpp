#include "MoveDemo.h"

// ========================== BestPractices 实现 ==========================

static string globalData; // 用于演示函数参数模式

void BestPractices::setDataCopy(const string& s) {
    globalData = s; // 拷贝赋值
    cout << "[拷贝版本] 设置数据: " << s << endl;
}

void BestPractices::setDataMove(string&& s) {
    globalData = std::move(s); // 移动赋值
    cout << "[移动版本] 设置数据: " << globalData << endl;
}

template<typename T>
void BestPractices::setDataPerfectForwarding(T&& s) {
    globalData = std::forward<T>(s);
    cout << "[完美转发] 设置数据: " << globalData << endl;
}

// 显式实例化模板
template void BestPractices::setDataPerfectForwarding<string&>(string&);
template void BestPractices::setDataPerfectForwarding<const string&>(const string&);
template void BestPractices::setDataPerfectForwarding<string&&>(string&&);

void BestPractices::demonstrateContainerOptimization() {
    cout << "\n=== 容器操作优化演示 ===" << endl;

    vector<ResourceManager> vec;
    vec.reserve(3); // 预分配空间，避免扩容时的移动

    ResourceManager obj("容器测试", 1000);

    cout << "\n--- push_back 拷贝版本 ---" << endl;
    vec.push_back(obj);

    cout << "\n--- push_back 移动版本 ---" << endl;
    ResourceManager obj2("移动测试", 1000);
    vec.push_back(std::move(obj2));

    cout << "\n--- emplace_back 原地构造（最优）---" << endl;
    vec.emplace_back("原地构造", 1000);

    cout << "\n容器最终状态:" << endl;
    for (size_t i = 0; i < vec.size(); ++i) {
        cout << "  元素 " << i << ": ";
        vec[i].printInfo();
    }
}

void BestPractices::demonstrateSmartPointerMove() {
    cout << "\n=== 智能指针转移演示 ===" << endl;

    // unique_ptr 移动 (C++11 兼容版本)
    unique_ptr<ResourceManager> ptr1(new ResourceManager("智能指针对象", 500));
    cout << "ptr1 创建完成，地址: " << ptr1.get() << endl;

    auto ptr2 = std::move(ptr1);
    cout << "移动后:" << endl;
    cout << "  ptr1 地址: " << ptr1.get() << " (应该是 nullptr)" << endl;
    cout << "  ptr2 地址: " << ptr2.get() << " (拥有对象)" << endl;

    if (ptr2) {
        cout << "  ptr2 指向的对象: ";
        ptr2->printInfo();
    }

    // shared_ptr 移动
    cout << "\n--- shared_ptr 移动演示 ---" << endl;
    shared_ptr<string> shared1(new string("共享字符串数据"));
    cout << "shared1 引用计数: " << shared1.use_count() << endl;

    auto shared2 = shared1; // 拷贝，引用计数增加
    cout << "拷贝后引用计数: " << shared1.use_count() << endl;

    auto shared3 = std::move(shared1); // 移动，不增加引用计数
    cout << "移动后:" << endl;
    cout << "  shared1 引用计数: " << (shared1 ? shared1.use_count() : 0) << endl;
    cout << "  shared3 引用计数: " << shared3.use_count() << endl;
}

ResourceManager BestPractices::createResource(const string& name, size_t size) {
    // 返回值优化 (RVO)，编译器自动优化，不需要显式 std::move
    return ResourceManager(name, size);
}

void BestPractices::runAllBestPractices() {
    MoveShowcase::printSectionHeader("最佳实践演示");

    cout << "\n=== 函数参数设计模式 ===" << endl;

    string testData = "测试数据";

    cout << "\n--- 拷贝版本调用 ---" << endl;
    setDataCopy(testData);
    cout << "调用后 testData: \"" << testData << "\" (保持不变)" << endl;

    cout << "\n--- 移动版本调用 ---" << endl;
    string moveData = "移动数据";
    setDataMove(std::move(moveData));
    cout << "调用后 moveData: \"" << moveData << "\" (被移动)" << endl;

    cout << "\n--- 完美转发调用 ---" << endl;
    string forwardData = "转发数据";
    setDataPerfectForwarding(forwardData); // 左值引用
    setDataPerfectForwarding(string("临时数据")); // 右值引用

    demonstrateContainerOptimization();
    demonstrateSmartPointerMove();

    cout << "\n=== 工厂函数最佳实践 ===" << endl;
    ResourceManager factoryResult = createResource("工厂对象", 2000);
    cout << "工厂创建的对象: ";
    factoryResult.printInfo();
}

// ========================== TrapExamples 实现 ==========================

void TrapExamples::trapBasicTypeMove() {
    cout << "\n=== 陷阱1：基本类型移动无效 ===" << endl;

    int a = 42;
    int b = std::move(a); // 对基本类型无效，等同于拷贝

    cout << "原始值 a: " << a << endl;
    cout << "移动值 b: " << b << endl;
    cout << "结论: 基本类型的移动等同于拷贝，没有性能提升" << endl;

    // 演示对基本类型指针的移动也是如此
    int* ptr1 = new int(100);
    int* ptr2 = std::move(ptr1); // 指针本身是基本类型

    cout << "指针移动:" << endl;
    cout << "  ptr1: " << ptr1 << " (仍然有效)" << endl;
    cout << "  ptr2: " << ptr2 << " (拷贝了指针值)" << endl;
    cout << "警告: 两个指针都指向同一内存，可能导致双重删除！" << endl;

    delete ptr1; // 只删除一次
    ptr2 = nullptr; // 避免意外使用
}

void TrapExamples::trapReturnValueMove() {
    cout << "\n=== 陷阱2：返回值错误使用move ===" << endl;

    // 错误做法的模拟
    auto badFactory = []() -> vector<int> {
        vector<int> v(1000, 42);
        // 错误！这会阻止返回值优化 (RVO)
        // return std::move(v);
        cout << "错误做法: return std::move(v) - 阻止RVO优化" << endl;
        return v; // 正确做法
    };

    auto goodFactory = []() -> vector<int> {
        vector<int> v(1000, 42);
        cout << "正确做法: return v - 允许RVO优化" << endl;
        return v; // 编译器自动优化
    };

    cout << "调用错误做法模拟:" << endl;
    vector<int> result1 = badFactory();

    cout << "调用正确做法:" << endl;
    vector<int> result2 = goodFactory();

    cout << "结论: 现代编译器会自动进行返回值优化，手动std::move反而有害" << endl;
}

void TrapExamples::trapUseAfterMove() {
    cout << "\n=== 陷阱3：移动后继续使用对象 ===" << endl;

    ResourceManager source("危险对象", 100);
    cout << "移动前: ";
    source.printInfo();

    ResourceManager target = std::move(source);
    cout << "移动后目标: ";
    target.printInfo();

    cout << "\n危险操作：尝试使用已移动的对象" << endl;
    cout << "移动后源对象: ";
    source.printInfo();

    // 虽然可以调用，但对象处于"有效但未指定"状态
    cout << "源对象是否有效: " << (source.isValid() ? "是" : "否") << endl;

    cout << "\n安全操作：可以重新赋值" << endl;
    source = ResourceManager("重新赋值", 50);
    cout << "重新赋值后: ";
    source.printInfo();

    cout << "结论: 移动后的对象只能销毁或重新赋值，不能访问其内容" << endl;
}

void TrapExamples::trapTemporaryObjectMove() {
    cout << "\n=== 陷阱4：临时对象多余move ===" << endl;

    auto processResource = [](ResourceManager res) {
        cout << "处理资源: ";
        res.printInfo();
    };

    cout << "多余的做法 - 临时对象使用std::move:" << endl;
    // 多余：临时对象本身就是右值
    processResource(std::move(ResourceManager("临时对象1", 100)));

    cout << "\n正确的做法 - 临时对象自动是右值:" << endl;
    // 正确：临时对象自动触发移动构造
    processResource(ResourceManager("临时对象2", 100));

    cout << "\n对比有名对象的移动:" << endl;
    ResourceManager named("有名对象", 100);
    processResource(std::move(named)); // 有名对象需要显式std::move

    cout << "结论: 只有左值（有名对象）需要std::move，临时对象不需要" << endl;
}

void TrapExamples::runAllTrapExamples() {
    MoveShowcase::printSectionHeader("常见陷阱演示");

    cout << "以下演示了使用std::move时的常见错误和陷阱：" << endl;

    trapBasicTypeMove();
    cout << "\n" << string(50, '-') << endl;

    trapReturnValueMove();
    cout << "\n" << string(50, '-') << endl;

    trapUseAfterMove();
    cout << "\n" << string(50, '-') << endl;

    trapTemporaryObjectMove();

    cout << "\n=== 陷阱总结 ===" << endl;
    cout << "1. 基本类型移动无效，仍然是拷贝" << endl;
    cout << "2. 返回值时不要使用std::move，会阻止RVO优化" << endl;
    cout << "3. 移动后的对象不能再使用，只能销毁或重新赋值" << endl;
    cout << "4. 临时对象不需要std::move，它们自动是右值" << endl;
}
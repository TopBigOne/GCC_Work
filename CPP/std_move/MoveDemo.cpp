#include "MoveDemo.h"
#include <algorithm>
#include <iomanip>

// ========================== ResourceManager 实现 ==========================

ResourceManager::ResourceManager()
    : name("Default"), data(), rawPtr(nullptr), size(0) {
    cout << "[构造] 默认构造函数: " << name << endl;
}

ResourceManager::ResourceManager(const string& name, size_t dataSize)
    : name(name), data(dataSize, 42), size(dataSize) {
    rawPtr = new int[size];
    for (size_t i = 0; i < size; ++i) {
        rawPtr[i] = static_cast<int>(i);
    }
    cout << "[构造] 带参构造函数: " << name << " (大小: " << dataSize << ")" << endl;
}

ResourceManager::~ResourceManager() {
    cout << "[析构] 析构函数: " << name << " (大小: " << size << ")" << endl;
    delete[] rawPtr;
    rawPtr = nullptr;
}

ResourceManager::ResourceManager(const ResourceManager& other)
    : name(other.name + "_copy"), data(other.data), size(other.size) {
    rawPtr = new int[size];
    std::copy(other.rawPtr, other.rawPtr + size, rawPtr);
    cout << "[拷贝构造] 拷贝构造函数: " << name << " <- " << other.name << endl;
}

ResourceManager::ResourceManager(ResourceManager&& other) noexcept
    : name(std::move(other.name)), data(std::move(other.data)),
      rawPtr(other.rawPtr), size(other.size) {
    // 清空源对象
    other.rawPtr = nullptr;
    other.size = 0;
    cout << "[移动构造] 移动构造函数: " << name << " <-- moved from source" << endl;
}

ResourceManager& ResourceManager::operator=(const ResourceManager& other) {
    cout << "[拷贝赋值] 拷贝赋值运算符: " << name << " = " << other.name << endl;
    if (this != &other) {
        // 清理当前资源
        delete[] rawPtr;

        // 拷贝新资源
        name = other.name + "_assigned";
        data = other.data;
        size = other.size;
        rawPtr = new int[size];
        std::copy(other.rawPtr, other.rawPtr + size, rawPtr);
    }
    return *this;
}

ResourceManager& ResourceManager::operator=(ResourceManager&& other) noexcept {
    cout << "[移动赋值] 移动赋值运算符: " << name << " <-- " << other.name << endl;
    if (this != &other) {
        // 清理当前资源
        delete[] rawPtr;

        // 移动新资源
        name = std::move(other.name);
        data = std::move(other.data);
        rawPtr = other.rawPtr;
        size = other.size;

        // 清空源对象
        other.rawPtr = nullptr;
        other.size = 0;
    }
    return *this;
}

void ResourceManager::printInfo() const {
    cout << "ResourceManager [" << name << "] - 数据大小: " << data.size()
         << ", 原始指针: " << (rawPtr ? "有效" : "空")
         << ", 大小: " << size << endl;
}

void ResourceManager::setName(const string& newName) {
    name = newName;
}

const string& ResourceManager::getName() const {
    return name;
}

size_t ResourceManager::getDataSize() const {
    return data.size();
}

bool ResourceManager::isValid() const {
    return !data.empty() || rawPtr != nullptr;
}

ResourceManager ResourceManager::createLarge(const string& name) {
    return ResourceManager(name, 10000);
}

// ========================== BasicDemo 实现 ==========================

void BasicDemo::demonstrateBasicMove() {
    cout << "\n=== 基本移动语义演示 ===" << endl;

    // 创建对象
    ResourceManager obj1("原始对象", 5);
    obj1.printInfo();

    cout << "\n--- 使用 std::move 转移所有权 ---" << endl;
    ResourceManager obj2 = std::move(obj1);

    cout << "\n移动后的状态:" << endl;
    cout << "源对象 obj1: ";
    obj1.printInfo();
    cout << "目标对象 obj2: ";
    obj2.printInfo();
}

void BasicDemo::demonstrateCopyVsMove() {
    cout << "\n=== 拷贝 vs 移动对比 ===" << endl;

    ResourceManager source("测试对象", 1000);

    cout << "\n--- 拷贝操作 ---" << endl;
    ResourceManager copied = source;

    cout << "\n--- 移动操作 ---" << endl;
    ResourceManager moved = std::move(source);

    cout << "\n最终状态:" << endl;
    cout << "原始对象: ";
    source.printInfo();
    cout << "拷贝对象: ";
    copied.printInfo();
    cout << "移动对象: ";
    moved.printInfo();
}

void BasicDemo::demonstrateStringMove() {
    cout << "\n=== 字符串移动演示 ===" << endl;

    string original = "这是一个很长的字符串，用来演示移动语义的效果";
    cout << "原始字符串: \"" << original << "\"" << endl;

    string moved = std::move(original);
    cout << "移动后:" << endl;
    cout << "  原始字符串: \"" << original << "\"" << endl;
    cout << "  移动字符串: \"" << moved << "\"" << endl;
}

void BasicDemo::demonstrateVectorMove() {
    cout << "\n=== 容器移动演示 ===" << endl;

    vector<int> original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    cout << "原始容器大小: " << original.size() << endl;

    vector<int> moved = std::move(original);
    cout << "移动后:" << endl;
    cout << "  原始容器大小: " << original.size() << endl;
    cout << "  移动容器大小: " << moved.size() << endl;
}

// ========================== PerformanceTest 实现 ==========================

void PerformanceTest::compareContainerPerformance() {
    cout << "\n=== 容器性能对比测试 ===" << endl;

    const size_t containerSize = 100000;
    const size_t innerSize = 100;

    // 创建测试数据
    vector<vector<int>> source(containerSize);
    for (auto& v : source) {
        v.resize(innerSize, 42);
    }

    auto start = chrono::high_resolution_clock::now();

    // 拷贝版本
    vector<vector<int>> copyVec = source;

    auto end = chrono::high_resolution_clock::now();
    auto copyTime = chrono::duration_cast<chrono::microseconds>(end - start);

    start = chrono::high_resolution_clock::now();

    // 移动版本
    vector<vector<int>> moveVec = std::move(source);

    end = chrono::high_resolution_clock::now();
    auto moveTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "容器拷贝时间 : " << copyTime.count() << " 微秒" << endl;
    cout << "容器移动时间 : " << moveTime.count() << " 微秒" << endl;
    cout << "性能提升    : " << (double)copyTime.count() / moveTime.count() << "x" << endl;
}

void PerformanceTest::compareStringPerformance() {
    cout << "\n=== 字符串性能对比测试 ===" << endl;

    const size_t iterations = 10000;
    string largeString(10000, 'A');

    auto start = chrono::high_resolution_clock::now();

    // 拷贝版本
    for (size_t i = 0; i < iterations; ++i) {
        string copy = largeString;
        (void)copy; // 避免编译器优化
    }

    auto end = chrono::high_resolution_clock::now();
    auto copyTime = chrono::duration_cast<chrono::microseconds>(end - start);

    // 重新创建字符串
    vector<string> sources;
    sources.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        sources.emplace_back(10000, 'A');
    }

    start = chrono::high_resolution_clock::now();

    // 移动版本
    for (size_t i = 0; i < iterations; ++i) {
        string moved = std::move(sources[i]);
        (void)moved; // 避免编译器优化
    }

    end = chrono::high_resolution_clock::now();
    auto moveTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "字符串拷贝时间: " << copyTime.count() << " 微秒" << endl;
    cout << "字符串移动时间: " << moveTime.count() << " 微秒" << endl;
    cout << "性能提升     : " << (double)copyTime.count() / moveTime.count() << "x" << endl;
}

void PerformanceTest::compareCustomObjectPerformance() {
    cout << "\n=== 自定义对象性能对比测试 ===" << endl;

    const size_t iterations = 1000;

    // 先预创建对象用于移动测试
    vector<ResourceManager> sources;
    sources.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        sources.emplace_back("移动测试", 10000);
    }

    auto start = chrono::high_resolution_clock::now();

    // 拷贝版本
    ResourceManager original("性能测试", 10000);
    vector<ResourceManager> copyResults;
    copyResults.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        copyResults.emplace_back(original); // 拷贝构造
    }

    auto end = chrono::high_resolution_clock::now();
    auto copyTime = chrono::duration_cast<chrono::microseconds>(end - start);

    start = chrono::high_resolution_clock::now();

    // 移动版本
    vector<ResourceManager> moveResults;
    moveResults.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        moveResults.emplace_back(std::move(sources[i])); // 移动构造
    }

    end = chrono::high_resolution_clock::now();
    auto moveTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "对象拷贝时间: " << copyTime.count() << " 微秒 (" << iterations << "次拷贝构造)" << endl;
    cout << "对象移动时间: " << moveTime.count() << " 微秒 (" << iterations << "次移动构造)" << endl;
    if (moveTime.count() > 0) {
        cout << "性能提升: " << (double)copyTime.count() / moveTime.count() << "x" << endl;
    } else {
        cout << "移动操作太快，无法精确测量" << endl;
    }

    cout << "说明: 预创建对象的时间不包含在移动测试中" << endl;
}

void PerformanceTest::runAllPerformanceTests() {
    MoveShowcase::printSectionHeader("性能测试");

    cout << "注意: 性能结果可能因编译器优化、系统负载等因素而变化" << endl;

    compareContainerPerformance();
    compareStringPerformance();
    compareCustomObjectPerformance();

    // 添加赋值操作的性能对比
    cout << "\n=== 赋值操作性能对比测试 ===" << endl;

    const size_t iterations = 1000;

    // 创建测试对象
    ResourceManager source1("拷贝赋值源", 5000);
    ResourceManager target1, target2;

    auto start = chrono::high_resolution_clock::now();

    // 拷贝赋值测试
    for (size_t i = 0; i < iterations; ++i) {
        target1 = source1; // 拷贝赋值
    }

    auto end = chrono::high_resolution_clock::now();
    auto copyAssignTime = chrono::duration_cast<chrono::microseconds>(end - start);

    start = chrono::high_resolution_clock::now();

    // 移动赋值测试 - 每次重新创建source避免重复移动空对象
    for (size_t i = 0; i < iterations; ++i) {
        ResourceManager tempSource("移动赋值源", 5000);
        target2 = std::move(tempSource); // 移动赋值
    }

    end = chrono::high_resolution_clock::now();
    auto moveAssignTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "拷贝赋值时间: " << copyAssignTime.count() << " 微秒" << endl;
    cout << "移动赋值时间: " << moveAssignTime.count() << " 微秒 (包含重新构造)" << endl;
    if (moveAssignTime.count() > 0) {
        cout << "注意: 移动赋值测试包含了对象重新构造的开销" << endl;
    }

    compareRealWorldScenario();

    // 添加性能差异分析
    explainPerformanceDifference();
}

void PerformanceTest::compareRealWorldScenario() {
    cout << "\n=== 真实场景：容器插入性能对比 ===" << endl;

    const size_t iterations = 1000;

    auto start = chrono::high_resolution_clock::now();

    // 拷贝版本：向容器插入大对象
    vector<ResourceManager> copyVector;
    ResourceManager templateObj("模板对象", 1000);

    for (size_t i = 0; i < iterations; ++i) {
        copyVector.push_back(templateObj); // 拷贝插入
    }

    auto end = chrono::high_resolution_clock::now();
    auto copyInsertTime = chrono::duration_cast<chrono::microseconds>(end - start);

    start = chrono::high_resolution_clock::now();

    // 移动版本：向容器插入大对象
    vector<ResourceManager> moveVector;

    for (size_t i = 0; i < iterations; ++i) {
        ResourceManager obj("移动对象" + to_string(i), 1000);
        moveVector.push_back(std::move(obj)); // 移动插入
    }

    end = chrono::high_resolution_clock::now();
    auto moveInsertTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "拷贝插入时间: " << copyInsertTime.count() << " 微秒" << endl;
    cout << "移动插入时间: " << moveInsertTime.count() << " 微秒 (包含对象构造)" << endl;

    // 纯移动测试：预先构造好的对象
    vector<ResourceManager> sources;
    sources.reserve(iterations);
    for (size_t i = 0; i < iterations; ++i) {
        sources.emplace_back("预构造对象" + to_string(i), 1000);
    }

    start = chrono::high_resolution_clock::now();

    vector<ResourceManager> pureMoveVector;
    for (size_t i = 0; i < iterations; ++i) {
        pureMoveVector.push_back(std::move(sources[i])); // 纯移动
    }

    end = chrono::high_resolution_clock::now();
    auto pureMoveTime = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << "纯移动插入时间: " << pureMoveTime.count() << " 微秒 (对象已预构造)" << endl;

    if (pureMoveTime.count() > 0) {
        cout << "拷贝 vs 纯移动性能提升: " << (double)copyInsertTime.count() / pureMoveTime.count() << "x" << endl;
    }

    cout << "\n关键洞察:" << endl;
    cout << "- 移动的优势在于避免深拷贝，而不是避免构造" << endl;
    cout << "- 当对象已存在时，移动比拷贝快很多" << endl;
    cout << "- 实际应用中要考虑对象的完整生命周期" << endl;
}

// ========================== MoveShowcase 实现 ==========================

void MoveShowcase::printSectionHeader(const string& title) {
    cout << "\n" << string(60, '=') << endl;
    cout << "  " << title << endl;
    cout << string(60, '=') << endl;
}

void MoveShowcase::printSubHeader(const string& title) {
    cout << "\n" << string(40, '-') << endl;
    cout << "  " << title << endl;
    cout << string(40, '-') << endl;
}

void MoveShowcase::pauseForUser() {
    cout << "\n按回车键继续..." << endl;
    cin.get();
}

void MoveShowcase::runCompleteDemo() {
    printSectionHeader("C++ std::move 完整演示程序");

    cout << "本程序将演示以下内容:" << endl;
    cout << "1. 基本移动语义概念" << endl;
    cout << "2. 性能对比测试" << endl;
    cout << "3. 最佳实践示例" << endl;
    cout << "4. 常见陷阱展示" << endl;

    pauseForUser();

    // 1. 基本演示
    printSectionHeader("基础概念演示");
    BasicDemo::demonstrateBasicMove();
    BasicDemo::demonstrateCopyVsMove();
    BasicDemo::demonstrateStringMove();
    BasicDemo::demonstrateVectorMove();

    pauseForUser();

    // 2. 性能测试
    PerformanceTest::runAllPerformanceTests();

    pauseForUser();

    // 3. 最佳实践
    BestPractices::runAllBestPractices();

    pauseForUser();

    // 4. 常见陷阱
    TrapExamples::runAllTrapExamples();

    printSectionHeader("演示完成");
    cout << "感谢观看 std::move 完整演示！" << endl;
}
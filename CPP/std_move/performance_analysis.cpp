#include "MoveDemo.h"

// 简化的测试对象，专门用于性能分析
class SimpleTestObject {
private:
    vector<int> data;
    string name;

public:
    SimpleTestObject(const string& n, size_t size) : name(n), data(size, 42) {}

    // 拷贝构造
    SimpleTestObject(const SimpleTestObject& other) : name(other.name), data(other.data) {}

    // 移动构造
    SimpleTestObject(SimpleTestObject&& other) noexcept : name(std::move(other.name)), data(std::move(other.data)) {}

    // 拷贝赋值
    SimpleTestObject& operator=(const SimpleTestObject& other) {
        if (this != &other) {
            name = other.name;
            data = other.data;
        }
        return *this;
    }

    // 移动赋值
    SimpleTestObject& operator=(SimpleTestObject&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            data = std::move(other.data);
        }
        return *this;
    }
};

// 添加到 PerformanceTest 命名空间
namespace PerformanceTest {
    void explainPerformanceDifference() {
        cout << "\n=== 🔍 性能差异分析 ===" << endl;

        const size_t iterations = 1000;
        const size_t objectSize = 10000;

        // 预创建对象用于移动测试
        vector<SimpleTestObject> sources;
        sources.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            sources.emplace_back("TestObj", objectSize);
        }

        SimpleTestObject templateObj("Template", objectSize);

        cout << "\n1. 纯拷贝构造测试:" << endl;
        auto start = chrono::high_resolution_clock::now();
        vector<SimpleTestObject> copyResults;
        copyResults.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            copyResults.emplace_back(templateObj); // 拷贝构造
        }
        auto end = chrono::high_resolution_clock::now();
        auto copyTime = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "   拷贝时间: " << copyTime.count() << " 微秒" << endl;

        cout << "\n2. 纯移动构造测试:" << endl;
        start = chrono::high_resolution_clock::now();
        vector<SimpleTestObject> moveResults;
        moveResults.reserve(iterations);
        for (size_t i = 0; i < iterations; ++i) {
            moveResults.emplace_back(std::move(sources[i])); // 移动构造
        }
        end = chrono::high_resolution_clock::now();
        auto moveTime = chrono::duration_cast<chrono::microseconds>(end - start);
        cout << "   移动时间: " << moveTime.count() << " 微秒" << endl;

        cout << "\n3. 性能对比:" << endl;
        if (moveTime.count() > 0) {
            double speedup = (double)copyTime.count() / moveTime.count();
            cout << "   移动比拷贝快: " << speedup << "x" << endl;
        } else {
            cout << "   移动操作太快，无法精确测量！" << endl;
        }

        cout << "\n4. 🎯 关键洞察:" << endl;
        cout << "   ❌ 错误测试设计：移动测试包含对象构造" << endl;
        cout << "   ✅ 正确测试设计：只比较纯移动 vs 纯拷贝" << endl;
        cout << "   📈 移动优势：避免深拷贝，不是避免构造" << endl;
        cout << "   🔧 实际应用：函数返回、容器插入、智能指针转移" << endl;
    }
}
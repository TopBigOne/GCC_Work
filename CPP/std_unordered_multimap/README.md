### unordered_multimap 解决了那些问题，应用场景是什么样的？


---

## 一、`unordered_multimap` 解决了哪些问题？

### 1. **一对多映射（同一个 key 对应多个 value）**

在实际开发中，经常遇到“**一个 key 可能对应多个 value**”的需求。比如：

- 一个班级有多个学生（key: 班级名，value: 学生ID）
- 一个单词出现在多篇文章（key: 单词，value: 文章ID）
- 一个错误码可能对应多条日志

如果用 `unordered_map`，同一个 key 只能存一个 value，后面的会覆盖前面的。而 `unordered_multimap` 允许**同一个 key 存储多个 value**，天然支持一对多。

### 2. **高效查找（哈希表 O(1) 平均复杂度）**
> std::unordered_multimap 底层就是用哈希表实现的
* C++标准库通常用开链法（Separate Chaining）的哈希表，也就是用一个桶数组（bucket array），每个桶是一个链表（有的实现可能用更高效的链表或链表+小vector）。

相比 `multimap`（基于红黑树，O(logN)），`unordered_multimap` 查找、插入和删除的平均复杂度为 O(1)，适合对性能有要求的场景。

### 3. **无需关心顺序**

有些场景只需要高效分组和查找，不关心元素顺序，哈希表（无序）比有序容器更合适。

---

## 二、典型应用场景

### 1. **反向索引**

- **搜索引擎**：建立“关键词 → 文档ID”索引，一个关键词可能出现在多篇文档。
- **标签系统**：标签 → 多个资源ID。

### 2. **日志归类**

- **日志分析**：错误码 → 多条日志，快速汇总某类错误的所有日志。

### 3. **分组存储**

- **学生分班**：班级名 → 多个学生ID。
- **产品分类**：类别 → 多个产品。

### 4. **事件分发/回调注册**

- **事件系统**：事件类型 → 多个回调函数/监听者。

### 5. **数据库多值字段**

- 比如数据库中某个字段允许多个值，内存中用 `unordered_multimap` 存储。

---

## 三、实际代码场景举例

### 例1：学生分班

```cpp
std::unordered_multimap<std::string, int> class_students;
class_students.insert({"ClassA", 1001});
class_students.insert({"ClassA", 1002});
class_students.insert({"ClassB", 2001});
// 查找ClassA的所有学生
auto range = class_students.equal_range("ClassA");
for (auto it = range.first; it != range.second; ++it) {
    std::cout << it->second << std::endl;
}
```

---

### 例2：日志归类

```cpp
std::unordered_multimap<int, std::string> error_logs;
error_logs.insert({404, "Page not found"});
error_logs.insert({404, "Resource missing"});
error_logs.insert({500, "Internal error"});
// 获取所有404错误日志
auto range = error_logs.equal_range(404);
for (auto it = range.first; it != range.second; ++it) {
    std::cout << it->second << std::endl;
}
```

---

### 例3：反向索引（关键词到文档）

```cpp
std::unordered_multimap<std::string, int> keyword_docs;
keyword_docs.insert({"C++", 1});
keyword_docs.insert({"C++", 2});
keyword_docs.insert({"AI", 2});
keyword_docs.insert({"AI", 3});
// 查找"AI"出现在哪些文档
auto range = keyword_docs.equal_range("AI");
for (auto it = range.first; it != range.second; ++it) {
    std::cout << it->second << std::endl;
}
```

---

## 四、总结

- **核心问题**：高效支持“一对多”映射，允许 key 重复，且查找、插入快。
- **典型场景**：反向索引、日志归类、分组存储、事件分发等。
- **适用条件**：不需要元素有序、key 支持哈希。

如果你有具体的业务需求，欢迎详细描述，我可以帮你分析适不适合用 `unordered_multimap`！




### forward_list 是 C++ 标准模板库（STL）中的一个容器，它实现了一个单向链表。这里是它的一些主要特点和使用场景：

### 特点
* 单向链表：forward_list 只支持单向遍历。每个元素只存储指向下一个元素的指针。
* 空间高效：由于只存储单个指针，forward_list 比双向链表（如 list）占用的空间更少。
* 插入和删除高效：在任意位置插入或删除元素的时间复杂度几乎是常数时间，不依赖于容器的大小。
* 不支持随机访问：不能像数组或向量那样直接通过索引访问元素。访问元素需要从头开始遍历。
###  使用场景
* 内存空间有限：当内存使用非常紧张且每个字节都很宝贵时，使用 forward_list 可以节省空间。
* 繁插入和删除操作：如果应用场景需要频繁在列表中插入或删除元素，特别是在头部附近操作，forward_list 是一个好选择。
* 不需要双向遍历：在不需要双向遍历元素的场景下，forward_list 比 list 更加高效。
* 性能关键型应用：在对时间和空间效率有严格要求的场景，如嵌入式系统或实时应用中，forward_list 可以提供更好的性能。


----
# 重点：forward_list :  是一个单向链表
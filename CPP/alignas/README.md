* 在 C++ 中可用于对齐要求的查询和显式指定。您可以根据需要使用它们来控制内存布局和对齐行为。
----
### alignof 和 alignas
* C++ 11 introduces two new keywords, alignof and alignas, to support control of memory alignment. 
* The alignof keyword can get a platform-dependent value of type std::size_t to query the alignment of the platform. 
* Of course, we are sometimes not satisfied with this, 
* and even want to customize the alignment of the structure. 
* Similarly, C++ 11 introduces alignas. To reshape the alignment of a structure”
----
* alignof 
  * 关键字用于获取给定类型的对齐要求。它返回一个 size_t 类型的值，表示所指定类型的对齐要求。
* alignas 
  * 关键字用于显式指定变量、类型或声明的对齐要求。它可以应用于变量、类型或成员声明
  * alignas 关键字不能用于函数声明或非静态数据成员。
---




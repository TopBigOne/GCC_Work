###   [tuple](https://blog.csdn.net/sevenjoin/article/details/88420885)
* 是一个固定大小的不同类型值的集合，是泛化的std::pair。
* tuple不支持迭代，只能通过元素索引(或tie解包)进行获取元素的值。但是给定的索引必须是在编译器就已经给定，不能在运行期进行动态传递，否则将发生编译错误：
---
* 我理解的tuple 是结构体的升级版本吧？
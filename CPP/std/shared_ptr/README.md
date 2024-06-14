###  第5章： [智能指针与内存管理](https://changkun.de/modern-cpp/zh-cn/05-pointers/#5-1-RAII-%E4%B8%8E%E5%BC%95%E7%94%A8%E8%AE%A1%E6%95%B0)
* std :: shared_ptr
> 一种智能指针，它能够记录多少个 shared_ptr 共同指向一个对象，从而消除显式的调用 delete，当引用计数变为零的时候就会将对象自动删除。
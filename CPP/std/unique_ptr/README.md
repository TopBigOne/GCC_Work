### unique_ptr 和 std::shared_ptr 的区别
1. 所有权管理：
   * std::unique_ptr：std::unique_ptr 是一种独占所有权的智能指针，它确保在其生命周期结束时，关联的对象会被自动删除。只能有一个 std::unique_ptr 指向同一个对象，它不能被拷贝，但可以通过移动语义转移所有权。
   * std::shared_ptr：std::shared_ptr 是一种共享所有权的智能指针。它可以让多个 std::shared_ptr 共同管理同一个对象，通过引用计数来追踪对象的所有权。只有当最后一个 std::shared_ptr 被销毁时，关联的对象才会被删除。
2. 线程安全性：
   * std::unique_ptr：std::unique_ptr 本身并不提供线程安全性。它适用于单线程环境或在多线程环境中通过适当的同步机制进行保护。
   * std::shared_ptr：std::shared_ptr 使用引用计数来管理对象的所有权，它可以在多个线程之间共享，因此在多线程环境中使用 std::shared_ptr 是线程安全的。
3. 内存开销：
   * std::unique_ptr：std::unique_ptr 通常比 std::shared_ptr 更轻量，因为它不需要维护引用计数。它的大小通常只是一个指针大小。
   * std::shared_ptr：std::shared_ptr 需要维护引用计数，这可能会增加一些内存开销。它的大小通常比一个指针大，并且引用计数的原子操作可能会带来一些性能开销。
### 根据不同的需求，您可以选择使用 std::unique_ptr 或 std::shared_ptr：
    > 当您需要独占所有权或管理动态分配的对象的生命周期时，应选择 std::unique_ptr。
    > 当您需要多个智能指针共享对同一个对象的所有权，并且需要在多线程环境中安全地共享对象时，应选择 std::shared_ptr。
    > 在性能要求较高的情况下，或者在不需要共享所有权的情况下，可以使用 std::unique_ptr 来避免额外的引用计数开销。
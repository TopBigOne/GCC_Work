#include <iostream>
#include <memory>

void foo(std::shared_ptr<int> i) {
    (*i)++;
}

int main() {

    auto pointer = std::make_shared<int>(10);
    foo(pointer);
    std::cout << "pointer.use_count()  = " << pointer.use_count() << std::endl;   // 3

    std::cout << *pointer << std::endl;

    auto pointer2 = pointer; // 引用计数+1
    std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 3

    auto pointer3 = pointer; // 引用计数+1
    std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 3
    int *p = pointer.get();  // 这样不会增加引用计数

    pointer2.reset();
    std::cout << "reset pointer2:" << std::endl;
    std::cout << "pointer.use_count()  = " << pointer.use_count() << std::endl;   // 2
    std::cout << "pointer2.use_count() = "<< pointer2.use_count() << std::endl;           // pointer2 已 reset; 0
    std::cout << "pointer3.use_count() = " << pointer3.use_count() << std::endl; // 2
    pointer3.reset();
    std::cout << "reset pointer3:" << std::endl;
    std::cout << "pointer.use_count() = " << pointer.use_count() << std::endl;   // 1
    std::cout << "pointer2.use_count() = " << pointer2.use_count() << std::endl; // 0
    std::cout << "pointer3.use_count() = "
              << pointer3.use_count() << std::endl;           // pointer3 已 reset; 0

    return 0;
}

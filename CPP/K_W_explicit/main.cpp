#include <iostream>


class MyClass {
public:
    explicit MyClass(int x) {
        this->x = x;
    }

    void display() {
        std::cout << "Value: " << x << std::endl;
    }

private:
    int x;
};

int main() {
    // case 1: ok
    MyClass myClass =  MyClass(5);
    myClass.display();

    // case 2: ok
    MyClass obj2(10); // OK
    obj2.display();

    // case3 ：不被允许
    // MyClass obj2 = 10; // Error: No implicit conversion allowed

    return 0;
}

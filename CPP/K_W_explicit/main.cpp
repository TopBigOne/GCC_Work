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
    MyClass myClass = MyClass(5);
    myClass.display();

    // case 2: ok
    MyClass obj2(10); // OK
    obj2.display();

    // case3 ：不被允许
    // MyClass obj2 = 10; // Error: No implicit conversion allowed


    int *a = new int;
    *a = 3;

    std::cout << "*a: " << *a << std::endl;
    std::cout << " a: " << a << std::endl;
    std::cout << " &a: " << &a << std::endl;
    std::cout << " *a++: " << *a++ << std::endl;
    std::cout << " *a++: " << *(a++) << std::endl;


    std::cout << "-----------------------------" << std::endl;
    char *temp = new char[1024];


    char *tempPtr = temp;

    *tempPtr++ = '1';
    *tempPtr++ = '2';
    *tempPtr   = '\0';


    std::cout << " tempPtr : " << temp << std::endl;
    std::cout << "*temp    : " << *temp << std::endl;

    std::cout << "&temp    : " << &temp << std::endl;
    std::cout << "&tempPtr : " << &tempPtr << std::endl;
    std::cout << "*tempPtr : " << *tempPtr << std::endl;


    return 0;
}

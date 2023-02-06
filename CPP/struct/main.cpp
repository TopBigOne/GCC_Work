#include <iostream>


struct Book {
    char *name;
    double price;

};
struct Address {
    char *name;
};

struct BookStore {
    Book book;
    Address address;

};

int main() {
    Book book{};
    book.name = "C++讲义";
    book.price = 12.6;

    return 0;
}

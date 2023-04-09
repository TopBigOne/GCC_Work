#include <iostream>
#include <math.h>

void test_one();

void test_two();

int main() {

     test_one();
    test_two();
    return 0;
}


void test_one() {
    char *str = nullptr;
    sprintf(str, "The PI value is: %s", "test_one");
    puts(str);
    puts(str);
    puts(str);
    fprintf(stderr, "the str value is : %s", str);
    fprintf(stderr, "the str value is : %s", str);

}

void test_two() {
    char str[80];
    sprintf(str, "The PI value is: %s", "test_two");
    puts(str);
    puts(str);
    puts(str);
    fprintf(stderr, "the str value is : %s", str);
    fprintf(stderr, "the str value is : %s", str);

}

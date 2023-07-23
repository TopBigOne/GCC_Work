#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Person {
    unsigned int  age;
    unsigned char *name;
} Person;


void test();

void test2();

void test3();


int main() {
    test();
    test2();
    test3();
    return 0;
}


void test() {
    Person person;
    if (person.name == NULL) {
        fprintf(stderr, "the name is NULL.");
        return;
    }
    person.name = malloc(sizeof(char));

    person.name[0]  = 'T';
    person.name[1]  = 'O';
    person.name[2]  = 'M';
    person.name[3]  = '1';
    person.name[4]  = '2';
    person.name[5]  = '3';
    person.name[6]  = '4';
    person.name[7]  = '5';
    person.name[6]  = '6';
    person.name[7]  = '7';
    person.name[8]  = '8';
    person.name[9]  = '9';
    person.name[10] = '0';
    person.name[11] = 'q';
    person.name[12] = 'a';
    person.name[13] = 'z';
    person.name[14] = 'x';
    person.name[15] = 's';
    person.name[16] = '-';
    person.name[17] = 0xAF;
    person.name[18] = 0x00;

    printf("person name :%s\n", person.name);
}


void test2() {
    char *temp = calloc(1, 20);

    Person person;
    person.name = temp + 10;
    person.name[0] = 'A';
    printf("person  name is %s\n", person.name);
    printf("person  name  length is %d\n", strlen(person.name));
    printf("person  name  length is %d\n", sizeof(person.name));
}

void test3() {
    char str[] = {0x00, 0x05, 'H', 'e', 'l', 'l', 'o'};
    // 从开头的两个字节中读取字符串的长度信息
    int  len   = (int) str[0] << 8 | str[1];
    printf("test3 : len is %d", len);
}


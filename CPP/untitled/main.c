#include <stdio.h>
#include <string.h>

void test1();
void test2();

void test1() {
    char first[] = "This is";
    char last[]  = "a potentially long string";
    int  r;
    int  size    = 16;
    char buffer[size];

    strcpy(buffer, first);
    printf("buffer : %s\n", buffer);
    r = strlcat(buffer, last, size);
    puts(buffer);
    printf("Value returned : %d\n", r);
    if (r > size) {
        puts("String truncated");
    } else {
        puts("String was fully copied");
    }
}

void test2(){
    puts("-------------------------------");
    char dest[20] = "Hello";
    const char * src = "   World";
    size_t dest_size = sizeof(dest);
    size_t  result = strlcat(dest,src,dest_size);
    printf("dest   : %s\n",dest);
    printf("result : %zu\n",result);
}


int main() {
    test1();
    test2();
    return 0;
}

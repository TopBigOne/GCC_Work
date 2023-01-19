#include <stdio.h>

#include <math.h>
#include "ifdef.h"
#include "doubincl.h"

#define message_for(a, b) \
  printf(#a" and "#b": we love you.\n") \

#define divide() printf("------------------------------------\n")

#define  tokenpaster(n) printf("token"#n"= %d\n ",token##n )


void test_1();

void test_2();

void test_3();

void test_4();

void test_5();

void test_6();

void test_7();

void test_8();

void test_9();

int main() {
    test_1();
    divide();
    test_2();
    divide();
    test_3();
    divide();
    test_4();
    divide();
    test_5();
    divide();
    test_6();
    divide();
    test_7();
    divide();
    test_8();
    divide();
    test_9();

    return 0;
}

void test_9() {
    start_test_double_include();
}

void test_8() {
    start_ifdef();
}

#define PR(X, ...) printf("Message " #X ":" __VA_ARGS__)

void test_7() {
    puts("变参宏：");
    puts("... 和 __VA_ARGS__");
    double x = 48;
    double y;
    y = sqrt(x);
    PR(1, "x = %g\n", x);
    PR(2, "x = %.2f,y = %4fn\n", x, y);


}

#define XNAME(n) w##n
#define PRINT_XN(n) printf("x" #n " = %d\n",w ## n)

void test_6() {
    puts("预处理器 粘合剂： ##运算符");
    int XNAME(9) = 0;//  int x1 = 14;
    int XNAME(2) = 0; // int x2 = 20;
    int w3 = 3;
    PRINT_XN(9);
    PRINT_XN(2);
    PRINT_XN(3);

}


#if !defined(xxx)
#define MESSAGE  "我 牛🐂B"
#define  MESSAGE2  "我 就是为了混口饭吃"
#define  MESSAGE3  "硬挺着....."
#endif

void test_4() {
    printf("Here is the message: %s\n", MESSAGE);
}

void test_3() {
    puts("标记粘贴运算符（##）");
    puts("---> 宏定义内的 标记粘贴运算符（##）会合并两个参数。它允许在宏定义中两个独立的标记被合并为一个标记.");

    int token35 = 40;
    tokenpaster(35);
}

void test_2() {
    message_for(1, 2);
    message_for(chenxiaoya, 陈筱雅);

}

void test_1() {
    printf("File :%s\n", __FILE__);
    printf("Date :%s\n", __DATE__);
    printf("Time :%s\n", __TIME__);
    printf("Line :%d\n", __LINE__);
    printf("ANSI :%d\n", __STDC__);
}

#define PSQR(x) printf("The square of " #x " is %d.\n",((x)*(x)))

void test_5() {
    puts("在字符串中替换：");
    int y = 5;
    PSQR(y); // # 就是y
    PSQR(2 + 4); //# 就是 2+4；

}

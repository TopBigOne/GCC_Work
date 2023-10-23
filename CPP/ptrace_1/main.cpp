#include <iostream>
#include <sys/ptrace.h>

int main() {
    int  a = 10;
    if (ptrace(PT_TRACE_ME, 0, 0, 0) == -1) {
        printf("don't trace me！\n");
        a = 30;
    }
    a = 45;
    printf("no one trace me！\n");
    return 0;
}

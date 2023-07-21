#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void test_fstat();

int main() {
    test_fstat();
    return 0;
}

void test_fstat() {
    int fd;
    struct stat sb;
    fd = open("/Users/dev/Documents/GCC_Work/CPP/fstat/README.md",O_RDONLY);
    if(fd<0){
        fprintf(stderr,"open file in failure");
        return;
    }
    if(fstat(fd,&sb)==-1){
        fprintf(stderr,"fstat in failure");
        return;
    }
    printf("File size : %lld bytes\n",sb.st_size);
    close(fd);
}

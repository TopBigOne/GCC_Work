#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define FILE_PATH "/Users/dev/Documents/GCC_Work/CPP/lseek/test.txt"


void my_code();

int main() {
    my_code();
    return 0;
}

void my_code() {
    int fd = open(FILE_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open in ERROR\n");
        return;
    }
    off_t off = lseek(fd, 0,SEEK_END);
    if (off == -1) {
        perror("lseek in ERROR\n");
        return;
    }
    printf("file size is %lld\n", off);
    close(fd);


}

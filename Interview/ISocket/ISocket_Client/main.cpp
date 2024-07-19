#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    std::cout << "客户端开始运行: " << std::endl;
    int                sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port        = htons(1234);

    connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    // read data;
    char buffer[256];
    read(sock, buffer, sizeof(buffer) - 1);

    std::cout << "  buffer : is " << buffer << std::endl;
    //关闭套接字
    close(sock);

    return 0;
}

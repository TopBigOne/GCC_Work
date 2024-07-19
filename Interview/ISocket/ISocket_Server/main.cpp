#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {

    std::cout << "socket 服务端开始运行" << std::endl;
    int                result    = -1;
    // SOCK_STREAM: 面向连接的数据传输方式
    int                serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // 将套接字 和IP，端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port        = htons(1234);
    result = bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (result < 0) {
        return 0;
    }
    // 开始进入监听
    listen(serv_sock, 20);

    // 接收客户端请求
    struct sockaddr_in clnt_addr;
    socklen_t          clnt_addr_size = sizeof(clnt_addr);
    int                clnt_sock      = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);

    // 向客户端发送数据
    auto toClientStr = "这是我给你的积极响应";
    write(clnt_sock, toClientStr, strlen(toClientStr));

    return 0;
}

//
// Created by yun.huang1 on 2023/6/7.
//

#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include "HttpServer.h"

HttpServer::HttpServer(int port, int l_mod, int trigmode, int conn_num) : TcpServer(port, l_mod, trigmode, conn_num) {

}


int HttpServer::recv_msg(int fd) {
    char buf[1024];
    if (m_server_trigmode == ET) {
        while (true) {
            ssize_t size = recv(fd, buf, 1024, 0);
            if (size < 0) {
                if (errno == EWOULDBLOCK) {
                    // 边缘触发模式下需要一次读取所有动态,所以存在多读取的情况,此处是多处理的情况
                    break;
                }
            }
            printf("recv msg:%s \n len:%zd\n", buf, size);
        }
    } else{
        ssize_t size = recv(fd, buf, 1024, 0);
        printf("recv msg LT :%s \n len:%zd\n", buf, size);
    }
//    write(fd, buf, strlen(buf));
    return 0;
}

int HttpServer::send_msg(int fd) {
    printf("%s\n", "write");
    // 构建 HTTP 响应报文
    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nReferrer-Policy: unsafe-url\r\n\r\n";
    const char *response = "<html><body><h1>Hello, world!</h1></body></html>";

    char http_response[1024];
    strcpy(http_response, header);
    strcat(http_response, response);

    ssize_t red = write(fd, http_response, strlen(http_response));

    close(fd);
    printf("write len = %zd\n", red);
    return 0;
}



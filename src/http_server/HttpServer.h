//
// Created by yun.huang1 on 2023/6/7.
//

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H

#include "TcpServer.h"


class HttpServer : public TcpServer {
    // todo 这个类应该包含socket 和epoll 同时能触发http的服务
    // todo 或者是这个类需要继承tcp,在这个类里面只实现http相关的业务
public:
    HttpServer() = default;

    HttpServer(int port, int l_mod, int trigmode, int conn_num);

    int recv_msg(int fd) override;

    int send_msg(int fd) override;

};


#endif //WEBSERVER_HTTPSERVER_H

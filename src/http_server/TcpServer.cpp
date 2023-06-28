//
// Created by yun.huang1 on 2023/5/31.
//
/**
 * TCP服务器通信基本流程
 * zhangyl 2018.12.13
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>


#include "TcpServer.h"

TcpServer::TcpServer(int port, int l_mod, int trigmode, int conn_num) : m_port(port),
                                                                        l_onoff(l_mod),
                                                                        m_server_trigmode(trigmode),
                                                                        m_conn_num(conn_num) {
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1) {
        std::cout << "create listen socket error." << std::endl;
        perror("socket");
    }
    // 设置延时关闭socket选项
    struct linger tmp = {l_onoff, L_LINGER};
    setsockopt(m_sockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    // 设置地址复用
    int flag = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    // 设置ip和地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);
    // 监听端口
    int ret = bind(m_sockfd, (struct sockaddr *) &address, sizeof(address));
    if (ret == -1) {
        std::cout << "create bind socket error." << std::endl;
        perror("bind");
    }
    // 等待客户端连接
    if (listen(m_sockfd, MAX_LISTEN) == -1) {
        std::cout << "create listen socket error." << std::endl;
        perror("listen");
    }
    // 设置I/O复用
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5); // 2.6.8版本以后该参数被废弃
    if (m_epollfd == -1) {
        std::cout << "epoll_create error." << std::endl;
        perror("epoll_create");
    }
    add_to_epoll(m_sockfd, false, m_server_trigmode);
}


// 将文件描述符添加到epoll实例中
int TcpServer::add_to_epoll(int fd, bool one_shot, int mode) {
    epoll_event event;
    event.data.fd = fd;
    if (mode == ET)
        set_ET_mod(event);
    else
        set_LT_mod(event);
    if (one_shot)
        set_one_shot_mod(event);
    epoll_add_fd(event, fd);
    return 0;

}

int TcpServer::setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

// 将文件描述符设置为非阻塞
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    return flags;
}

void TcpServer::epoll_del_fd(int fd) {
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void TcpServer::set_one_shot_mod(epoll_event &event) {
    // EPOLLONESHOT 触发一次就要重新添加
    event.events |= EPOLLONESHOT;

}

void TcpServer::set_LT_mod(epoll_event &event) {
    event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;

}

void TcpServer::set_ET_mod(epoll_event &event) {
    // EPOLLIN 可读时通知
    // EPOLLOUT 可写时通知
    // EPOLLRDHUP 检测对方是否异常退出
    // EPOLLET 设置边缘触发模式
    event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;

}

void TcpServer::epoll_add_fd(epoll_event &event, int fd) {
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void TcpServer::enevt_loop() {
    bool stop_server = false;
    while (!stop_server) {
        int nfds = epoll_wait(m_epollfd, m_event, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        // 处理所有就绪事件
        for (int i = 0; i < nfds; ++i) {
            //接收新用户的连接
            if (m_event[i].data.fd == m_sockfd) {
                add_conn();
            } else {
                if (m_event[i].events & EPOLLIN) {
                    // 接收消息  如果消息长度为0,则关闭连接,取消epoll注册,关闭fd连接
                    printf("%s","EPOLLIN  recv\n");
                    recv_msg(m_event[i].data.fd);
                    // 接收完消息需要重新注册到epoll里面
                    add_to_epoll(m_event[i].data.fd, true, m_server_trigmode);
                } else if (m_event[i].events & EPOLLOUT) {
                    printf("%s","EPOLLOUT  send\n");
                    send_msg(m_event[i].data.fd);
                } else if (m_event[i].events & EPOLLRDHUP) {
                    close_conn(m_event[i].data.fd);
                }
            }
        }
    }

}

int TcpServer::add_conn() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if (m_server_trigmode == LT) {
        int clientfd = accept(m_sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (clientfd == -1) {
            perror("accept");
            return -1;
        }
        if (m_conn_num >= MAX_FD) {
            printf("%s", "Internal server busy");
            char buf[] = "Internal server busy";
            send(clientfd, buf, strlen(buf), 0);
            close(clientfd);
            return -1;
        }
        printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        add_to_epoll(clientfd, true, m_server_trigmode);
    } else {
        while (true) {
            int clientfd = accept(m_sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
            if (clientfd == -1) {
                if (errno == EWOULDBLOCK) {
                    // EWOULDBLOCK异常表示所有的请求都处理完毕,因为在边缘触发模式下,每次需要把该fd下所有的请求都处理掉,所以需要做处理
                    break;
                }
                perror("accept");
                return -1;
            }
            if (m_conn_num >= MAX_FD) {
                printf("%s", "Internal server busy");
                char buf[] = "Internal server busy";
                send(clientfd, buf, strlen(buf), 0);
                close(clientfd);
                return -1;
            }
            printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            add_to_epoll(clientfd, true, m_server_trigmode);
        }
    }
    return 0;
}


int TcpServer::close_conn(int fd) {
    // 关闭套接字并从 epoll 实例中移除
    close(fd);
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL);
    return 0;
}

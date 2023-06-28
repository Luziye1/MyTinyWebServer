//
// Created by yun.huang1 on 2023/5/31.
//

#ifndef WEBSERVER_TCPSERVER_H
#define WEBSERVER_TCPSERVER_H

#include <list>
#include <sys/epoll.h>

using namespace std;

// 关闭套接字时,延时等待数据发送的时间
#define L_LINGER 1
// 设置listen的最大数量
#define MAX_LISTEN 20
// 最大事件数
#define MAX_EVENT_NUMBER 10000
// 水平触发模式
#define LT 0
// 边缘触发模式
#define ET 1
// epoll事件最大触发数
#define MAX_EVENTS 10
// 最大文件描述符
#define MAX_FD 65536


class TcpServer {
public:
    int m_port; // 监听端口
    int m_sockfd; // 服务端文件描述符
    int m_epollfd; // epoll文件描述符
    int l_onoff; // 0禁用延时关闭,1启用延时关闭
    int m_server_trigmode; // 0服务端水平模式,1服务端边缘模式
    int m_conn_num; // 连接数量
    struct epoll_event m_event[MAX_EVENTS]; // epoll事件
    list<int> m_connfd_list; //所有连接的文件描述符列表

    TcpServer() = default;

    TcpServer(int port, int l_mod, int trigmode, int conn_num);

    // 添加文件描述符到epoll里面
    int add_to_epoll(int fd, bool one_shot, int mode);

    // 设置水平触发模式
    void set_LT_mod(epoll_event &event);

    // 设置边缘触发模式
    void set_ET_mod(epoll_event &event);

    // 设置单次提醒模式
    void set_one_shot_mod(epoll_event &event);

    // 添加一个描述符到epoll
    void epoll_add_fd(epoll_event &event, int fd);

    // 从epoll删除一个描述符
    void epoll_del_fd(int fd);

    // 设置文件描述符非阻塞
    int setnonblocking(int fd);

    // 添加一个新的连接
    int add_conn();

    // 接收消息
    virtual int recv_msg(int fd) = 0;

    // 发送消息
    virtual int send_msg(int fd) = 0;

    // 断开一个连接
    int close_conn(int fd);

    // 循环接收消息
    void enevt_loop();


private:

};


#endif //WEBSERVER_TCPSERVER_H


#ifndef MY_PROJECT_MYSQLCONNPOOL_H
#define MY_PROJECT_MYSQLCONNPOOL_H

#include <mysql/mysql.h>
#include <list>
#include <string>
#include <mutex>

using namespace std;

class MysqlConnPool {
public:
    MysqlConnPool() = default;

    MysqlConnPool(const string &host, const string &user, const string &password, const string &database,
              uint32_t max_connections);

    // 析构函数,释放所有数据库连接
    ~MysqlConnPool();

    // 获取一个数据库连接
    MYSQL *get_connection();

    // 释放一个已经使用完毕的数据库连接
    void release_connection(MYSQL *conn);

private:
    // 初始化连接池
    void init();

    // 创建一个新的数据库连接
    MYSQL *create_connection();

private:
    string host_;                     // 数据库主机名
    string user_;                     // 数据库用户名
    string password_;                 // 数据库用户密码
    string database_;                 // 数据库名
    uint32_t max_connections_;        // 连接池最大连接数
    uint32_t cur_connections_;        // 当前已经创建的连接数
    list<MYSQL *> conn_list_;         // 数据库连接列表
    mutex mutex_;                     // 互斥锁，用于线程安全访问连接池

};


#endif //MY_PROJECT_MYSQLCONNPOOL_H

#include <mutex>
#include "MysqlConnPool.h"

using namespace std;

MysqlConnPool::MysqlConnPool(const string &host, const string &user, const string &password, const string &database,
                             uint32_t max_connections) : host_(host), user_(user), password_(password),
                                                         database_(database),
                                                         max_connections_(max_connections), cur_connections_(0) {
    init();
}

MysqlConnPool::~MysqlConnPool() {
    for (auto it = conn_list_.begin(); it != conn_list_.end(); ++it) {
        mysql_close(*it);
    }
}

MYSQL *MysqlConnPool::get_connection() {
    unique_lock<mutex> lock(mutex_);
    while (conn_list_.empty()) {
        if (cur_connections_ >= max_connections_) {
            return nullptr;
        }
        MYSQL *conn = create_connection();
        if (conn == nullptr) {
            return nullptr;
        }
        conn_list_.push_back(conn);
        ++cur_connections_;
    }
    MYSQL *conn = conn_list_.front();
    conn_list_.pop_front();
    return conn;
}

void MysqlConnPool::release_connection(MYSQL *conn) {
    unique_lock<mutex> lock(mutex_);
    conn_list_.push_back(conn);
}

void MysqlConnPool::init() {
    for (int i = 0; i < max_connections_; ++i) {
        MYSQL *conn = create_connection();
        if (conn == nullptr) {
            break;
        }
        conn_list_.push_back(conn);
        ++cur_connections_;
    }
}

MYSQL *MysqlConnPool::create_connection() {
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) {
        return nullptr;
    }
    if (mysql_real_connect(conn, host_.c_str(), user_.c_str(), password_.c_str(),
                           database_.c_str(), 0, nullptr, 0) == nullptr) {
        mysql_close(conn);
        return nullptr;
    }
    return conn;
}

cmake_minimum_required(VERSION 3.6.2)

# 项目名称
set(PROJECT_NAME webserver)
project(${PROJECT_NAME})

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# 头文件路径
set(MY_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src/mysql_conn_pool
        ${PROJECT_SOURCE_DIR}/src/http_server
        ${PROJECT_SOURCE_DIR}/src/thread_pool
        /usr/include
        )

# 依赖库路径
set(MY_LIBRARY_DIRS
        /usr/lib64/mysql
        )

# 定义库依赖
set(MY_LIBRARIES
        mysqlclient
        pthread
        )

# 添加头文件路径和链接库路径
include_directories(${MY_INCLUDE_DIRS})
link_directories(${MY_LIBRARY_DIRS})

# 添加可执行文件
add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/src/mysql_conn_pool/MysqlConnPool.cpp
        ${PROJECT_SOURCE_DIR}/src/http_server/TcpServer.cpp
        ${PROJECT_SOURCE_DIR}/src/http_server/HttpServer.cpp
        ${PROJECT_SOURCE_DIR}/src/thread_pool/ThreadPool.cpp
        )

# 链接库
target_link_libraries(${PROJECT_NAME} ${MY_LIBRARIES})

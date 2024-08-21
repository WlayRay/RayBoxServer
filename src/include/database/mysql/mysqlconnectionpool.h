#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "mysqlconnection.h"
class MysqlConnectionPool {
public:
    static MysqlConnectionPool& getInstance(
        const std::string& configFilePath = "mysql.cnf");

    std::shared_ptr<MysqlConnection> getConnection();

private:
    MysqlConnectionPool(const std::string& configFilePath = "mysql.cnf");
    MysqlConnectionPool(const MysqlConnectionPool&) = delete;
    MysqlConnectionPool(MysqlConnectionPool&&) = delete;

    // 加载配置文件，读取数据库连接配置
    bool loadConfigFile(std::string configFilePath);

    // 运行在独立的线程中，专门负责生产新连接
    void produceConnectionTask();

    // 运行在独立线程中，扫描连接池中的所有连接，释放超出最长空闲时间的连接
    void releaseIdleConnection();

    // 运行在独立线程中，用于定时发送心跳消息，保证连接始终可用
    // 如果不发送心跳，那么连接在服务端空闲时间达到wait_timeout之后，将不可用，引发Lost connection to MySQL server during query异常
    void keepAlive();
private:
    std::string _ip;        // 数据库ip地址
    unsigned short _port;   // 数据库端口号
    std::string _username;  // 数据库用户名
    std::string _password;  // 数据库密码
    std::string _dbname;    // 数据库名称
    std::string _charset;   // 数据库字符集

    int _initSize;           // 连接池初始连接数
    int _maxSize;            // 连接池最大连接数
    int _maxIdleTime;        // 连接最长空闲时间
    int _connectionTimeout;  // 获取连接的超时时间

    std::queue<MysqlConnection*> _connectionQueue;  // 存放数据库连接
    std::mutex _queueMtx;  // 保证连接队列的互斥访问
    std::condition_variable _cv;
};

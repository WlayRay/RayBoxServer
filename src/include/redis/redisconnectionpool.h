#pragma once
#include <sw/redis++/redis++.h>
#include <sw/redis++/subscriber.h>

#include <string>

class RedisConnectionPool
{
public:
    void publish(std::string channel, std::string msg);
    sw::redis::Subscriber &getSubscriber();
    static RedisConnectionPool &instance();
    RedisConnectionPool(const RedisConnectionPool &) = delete;
    RedisConnectionPool(RedisConnectionPool &&) = delete;

private:
    bool loadConfigFile(std::string configFilePath);
    RedisConnectionPool(const std::string &configFilePath = "redis.cnf");

private:
    std::string _ip;       // redis服务IP
    unsigned short _port;  // redis端口号
    std::string _username; // redis用户名
    std::string _password; // redis密码
    std::string _db;       // redis数据库

    int _size;                 // 连接池初始连接数
    int _connection_idle_time; // 连接最大空闲时间
    int _wait_timeout;         // 获取连接的超时时间

    sw::redis::Redis _redis{"tcp://127.0.0.1:6379"};
};


#include <iostream>
#include <thread>

#include "redisconnectionpool.h"

int main()
{
    // 订阅redis消息
    sw::redis::Subscriber &sub =
        RedisConnectionPool::instance().getSubscriber();
    sub.psubscribe("raybox-user-*");
    // 设置回调函数
    sub.on_pmessage(
        [](std::string pattern, std::string channel, std::string msg)
        {
            std::string str = channel;
            std::string prefix = "raybox-user-";
            int userId;

            if (str.substr(0, prefix.size()) == prefix)
            {
                userId = std::stoi(str.substr(prefix.size()));
                std::cout << userId << ":" << msg << std::endl;
            }
            else
            {
                // 非相关通道下的消息
            }
        });
    // 开启单独的子线程消费
    std::thread t([&]()
                  {
        while (true) {
            try {
                sub.consume();
            } catch (const sw::redis::Error& err) {
                std::cout << err.what() << std::endl;
            }
        } });
    t.detach();

    RedisConnectionPool::instance().publish("raybox-user-123", "hello, 123");
    RedisConnectionPool::instance().publish("raybox-user-456", "hello, 456");
    RedisConnectionPool::instance().publish("123456", "hello, 123456");
}
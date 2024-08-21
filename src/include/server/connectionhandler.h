#pragma once
#include <muduo/net/TcpConnection.h>

#include <mutex>
#include <unordered_map>

// 处理用户和TcpConnection的映射
class ConnectionHandler
{
public:
    // 添加一个在线用户的TcpConnection
    void addConnection(int userId, muduo::net::TcpConnectionPtr tcpconn);
    // 删除一个在线用户的TcpConnection(用于异常断开连接时)
    int delConnection(muduo::net::TcpConnectionPtr tcpconn);
    // 删除一个在线用户的TcpConnection(用于用户正常退出登录时)
    int delConnection(int userId);

    // 转发消息
    bool forwardMsg(int toUserId, std::string msg);

    // 单例模式，保证线程安全
    static ConnectionHandler &instance();
    ConnectionHandler(const ConnectionHandler &) = delete;
    ConnectionHandler(ConnectionHandler &&) = delete;

private:
    // 单例，考虑线程安全
    ConnectionHandler();

    // 存储在线用户的TcpConnection
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;
    // 保证_userConnMap的线程安全，互斥访问
    std::mutex _connMutex;
};
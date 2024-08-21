#include "connectionhandler.h"

#include <mutex>
// 添加一个在线用户的TcpConnection
void ConnectionHandler::addConnection(int userId,
                                      muduo::net::TcpConnectionPtr tcpconn)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    // _userConnMap.insert({userId, tcpconn});
    _userConnMap[userId] = tcpconn;
}

// 删除一个即将离线用户的TcpConnection (用于异常断开连接时)
int ConnectionHandler::delConnection(muduo::net::TcpConnectionPtr tcpconn)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
    {
        if (it->second == tcpconn)
        {
            int userId = it->first;
            _userConnMap.erase(it);
            return userId;
        }
    }
    return -1; // 未知的TcpConnection
}

// 删除一个在线用户的TcpConnection(用于用户正常退出登录时)
int ConnectionHandler::delConnection(int userId)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(userId);
    if (it != _userConnMap.end())
    {
        _userConnMap.erase(it);
        return userId;
    }
    return -1; // 未知的用户Id
}

// 转发消息到特定的用户
bool ConnectionHandler::forwardMsg(int toUserId, std::string msg)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(toUserId);
    if (it != _userConnMap.end())
    {
        // 该用户在本服务器登录
        it->second->send(msg);
        return true;
    }
    else
    {
        // 该用户不在本服务器登录，或该用户未登录
        return false;
    }
}

// 获取一个实例对象，线程安全
ConnectionHandler &ConnectionHandler::instance()
{
    // 静态局部，底层汇编自动加锁解锁，保证线程安全
    static ConnectionHandler connectionHandler;
    return connectionHandler;
}

ConnectionHandler::ConnectionHandler()
{
    // do nothing
}
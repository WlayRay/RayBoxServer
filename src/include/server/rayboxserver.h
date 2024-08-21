#pragma once

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

class RayBoxServer
{
public:
    RayBoxServer(muduo::net::EventLoop *loop,
                 const muduo::net::InetAddress &listenAddr,
                 const std::string &name);

    void start();

private:
    // TODO 考虑用户消息加密
    void onConnection(const muduo::net::TcpConnectionPtr &conn);

    void onMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer, muduo::Timestamp timestamp);

    muduo::net::TcpServer _server;
    muduo::net::EventLoop *_loop;
};
#pragma once

#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>

#include <json.hpp>

#include "usermodel.h"

class UserService {
public:
    // 处理用户登录
    void login(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
               muduo::Timestamp time);

    // 处理用户注册
    void reg(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
             muduo::Timestamp time);

    // 用户正常退出
    void logout(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
                muduo::Timestamp time);

    // 客户端异常断开连接(由muduo回调，不是根据消息类型回调)
    // TODO
    // 考虑网络异常，用户已下线但连接依然存在的情况（解决办法：采用心跳机制）
    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);

    // 服务器主动关闭时回调（响应CTRL+C事件，不是根据消息类型回调）
    void logoutAll();

    // 单例，考虑线程安全
    static UserService &instance();
    UserService(const UserService &) = delete;
    UserService(UserService &&) = delete;

private:
    UserService();
    // 对持久层的操作接口
    UserModel _userModel;
};
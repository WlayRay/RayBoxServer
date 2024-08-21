#pragma once
#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>

#include <json.hpp>

#include "offlinemessagemodel.h"
#include "usermodel.h"
class UserChatService {
public:
    void userChat(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
               muduo::Timestamp time);

    static UserChatService &instance();
    UserChatService(const UserChatService &) = delete;
    UserChatService(UserChatService &&) = delete;

private:
    UserChatService();

    OfflineMsgModel _offlineMsgModel;
    UserModel _userModel;
};
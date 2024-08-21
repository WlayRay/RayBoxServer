#pragma once
#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>

#include <json.hpp>

#include "groupmodel.h"
#include "offlinemessagemodel.h"
#include "usermodel.h"

class GroupChatService {
public:
    void createGroup(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time);
    void groupQuery(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time);
    void addGroup(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time);
    void groupChat(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time);
    void groups(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time);

    static GroupChatService& instance();
    GroupChatService(const GroupChatService&) = delete;
    GroupChatService(GroupChatService&&) = delete;

private:
    GroupChatService();

    OfflineMsgModel _offlineMsgModel;
    UserModel _userModel;
    GroupModel _groupModel;
};
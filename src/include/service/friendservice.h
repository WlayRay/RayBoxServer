#pragma once

#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>

#include <json.hpp>

#include "friendmodel.h"
#include "offlinemessagemodel.h"
#include "usermodel.h"
#include "groupmodel.h"

class FriendService {
public:
    void addFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
                   muduo::Timestamp time);
    void requestAddFriend(const muduo::net::TcpConnectionPtr &conn,
                          nlohmann::json &js, muduo::Timestamp time);

    void delFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
                   muduo::Timestamp time);
    void allFriends(const muduo::net::TcpConnectionPtr &conn,
                    nlohmann::json &js, muduo::Timestamp time);
    void searchFriends(const muduo::net::TcpConnectionPtr &conn,
                    nlohmann::json &js, muduo::Timestamp time);
    
    static FriendService &instance();
    FriendService(const FriendService &) = delete;
    FriendService(FriendService &&) = delete;

private:
    FriendService();
    OfflineMsgModel _offlineMsgModel;
    UserModel _userModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
};
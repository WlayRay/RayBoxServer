#include "msgdispatcher.h"

#include <functional>

#include "friendservice.h"
#include "groupchatservice.h"
#include "msgtypeenum.h"
#include "offlinemsgservice.h"
#include "userchatservice.h"
#include "userservice.h"

MsgDispatcher &MsgDispatcher::instance()
{
    static MsgDispatcher msgDispatcher;
    return msgDispatcher;
}

MsgHandler MsgDispatcher::getMsgHandler(MsgTypeEnum msgTypeEnum)
{
    auto it = _msgHandlerMap.find(msgTypeEnum);
    if (it == _msgHandlerMap.end())
    { // 未找到合适的回调
        return _msgHandlerMap[MsgTypeEnum::METHOD_NOT_FOUND];
    }
    else
    {
        return it->second;
    }
}

MsgDispatcher::MsgDispatcher()
{
    // 注册消息回调函数

    // 登录
    _msgHandlerMap[MsgTypeEnum::USER_LOGIN] = std::bind(
        &UserService::login, std::ref(UserService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 注册
    _msgHandlerMap[MsgTypeEnum::USER_REG] = std::bind(
        &UserService::reg, std::ref(UserService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 注销
    _msgHandlerMap[MsgTypeEnum::USER_LOGOUT] = std::bind(
        &UserService::logout, std::ref(UserService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    // 用户聊天消息
    _msgHandlerMap[MsgTypeEnum::USER_CHAT] = std::bind(
        &UserChatService::userChat, std::ref(UserChatService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    // 请求添加好友消息
    _msgHandlerMap[MsgTypeEnum::FRIEND_REQUEST] = std::bind(
        &FriendService::requestAddFriend, std::ref(FriendService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 添加好友关系
    _msgHandlerMap[MsgTypeEnum::FRIEND_Add] = std::bind(
        &FriendService::addFriend, std::ref(FriendService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 删除好友关系
    _msgHandlerMap[MsgTypeEnum::FRIEND_DELETE] = std::bind(
        &FriendService::delFriend, std::ref(FriendService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 查询好友关系
    _msgHandlerMap[MsgTypeEnum::FRIEND_QUERY] = std::bind(
        &FriendService::allFriends, std::ref(FriendService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 查询账户
    _msgHandlerMap[MsgTypeEnum::ACCOUNT_QUERY] = std::bind(
        &FriendService::searchFriends, std::ref(FriendService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    // 群聊消息
    _msgHandlerMap[MsgTypeEnum::GROUP_CHAT] = std::bind(
        &GroupChatService::groupChat, std::ref(GroupChatService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 加群
    _msgHandlerMap[MsgTypeEnum::GROUP_ADD] = std::bind(
        &GroupChatService::addGroup, std::ref(GroupChatService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 查询所有群
    _msgHandlerMap[MsgTypeEnum::GROUP_QUERY] = std::bind(
        &GroupChatService::groups, std::ref(GroupChatService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 创建群
    _msgHandlerMap[MsgTypeEnum::GROUP_CREATE] = std::bind(
        &GroupChatService::createGroup, std::ref(GroupChatService::instance()),
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    // 根据群名查询
    _msgHandlerMap[MsgTypeEnum::GROUP_QUERY_BY_NAME] =
        std::bind(&GroupChatService::groupQuery,
                  std::ref(GroupChatService::instance()), std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);

    // 查询离线消息
    _msgHandlerMap[MsgTypeEnum::OFFLINE_MSG_QUERY] =
        std::bind(&OfflineMsgService::getAllOfflineMsgs,
                  std::ref(OfflineMsgService::instance()), std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3);

    // 常规响应
    _msgHandlerMap[MsgTypeEnum::NORMAL_ACK] =
        [](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
           muduo::Timestamp timestamp)
    {
        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
        response["errno"] = 200;
        conn->send(response.dump());
    };

    // JSON字符串解析错误
    _msgHandlerMap[MsgTypeEnum::PARSE_JSON_ERROR] =
        [](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
           muduo::Timestamp timestamp)
    {
        nlohmann::json response;
        response["msgType"] =
            static_cast<int>(MsgTypeEnum::PARSE_JSON_ERROR);
        response["errno"] = 401;
        response["errmsg"] = "Failed to parse the json string!";
        conn->send(response.dump());
    };
    // 未找到请求的回调函数
    _msgHandlerMap[MsgTypeEnum::METHOD_NOT_FOUND] =
        [](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
           muduo::Timestamp timestamp)
    {
        nlohmann::json response;
        response["msgType"] =
            static_cast<int>(MsgTypeEnum::METHOD_NOT_FOUND);
        response["errno"] = 404;
        response["errmsg"] =
            "No such method, failed to process your request!";
        conn->send(response.dump());
    };
    // 服务器出错
    _msgHandlerMap[MsgTypeEnum::SERVER_PROCESS_ERROR] =
        [](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
           muduo::Timestamp timestamp)
    {
        nlohmann::json response;
        response["msgType"] =
            static_cast<int>(MsgTypeEnum::SERVER_PROCESS_ERROR);
        response["errno"] = 500;
        response["errmsg"] = "server error, try again later!";
        conn->send(response.dump());
    };
}
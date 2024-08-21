#include "friendservice.h"

#include <string>
#include <unordered_map>

#include "connectionhandler.h"
#include "msgdispatcher.h"
#include "msgtypeenum.h"
#include "redisconnectionpool.h"

void FriendService::addFriend(const muduo::net::TcpConnectionPtr &conn,
                              nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("U1") && js.contains("U2"))
    {
        int u1 = js["U1"].get<int>();
        int u2 = js["U2"].get<int>();
        _friendModel.insert(u1, u2);
        _friendModel.insert(u2, u1);
        User user1 = _userModel.query(u1);
        User user2 = _userModel.query(u2);

        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::FRIEND_Add);
        response["errno"] = 200;
        response["userId"] = u2;
        response["name"] = user2.name();
        conn->send(response.dump());

        nlohmann::json response2;
        response2["msgType"] = static_cast<int>(MsgTypeEnum::FRIEND_Add);
        response2["errno"] = 200;
        response2["userId"] = u1;
        response2["name"] = user1.name();
        response2["new"] = true;
        if (!ConnectionHandler::instance().forwardMsg(u2, response2.dump()))
        {
            // 不在本服务器或未登录
            if (user2.state() == "offline")
            {
                // 用户不在线，转存为离线消息
                _offlineMsgModel.insert(u2, response2.dump());
            }
            else
            {
                // 用户不在本服务器
                RedisConnectionPool::instance().publish(
                    "RayBox-user-" + std::to_string(u2), response2.dump());
            }
        }
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}
void FriendService::requestAddFriend(const muduo::net::TcpConnectionPtr &conn,
                                     nlohmann::json &js,
                                     muduo::Timestamp time)
{
    if (js.contains("TOID"))
    {
        int toUserId = js["TOID"].get<int>();
        if (!ConnectionHandler::instance().forwardMsg(toUserId, js.dump()))
        {
            // 不在本服务器或未登录
            User user = _userModel.query(toUserId);
            if (user.state() == "offline")
            {
                // 用户不在线，转存为离线消息
                _offlineMsgModel.insert(toUserId, js.dump());
            }
            else
            {
                // 用户不在本服务器
                RedisConnectionPool::instance().publish(
                    "RayBox-user-" + std::to_string(toUserId), js.dump());
            }
        }
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

void FriendService::delFriend(const muduo::net::TcpConnectionPtr &conn,
                              nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID") && js.contains("TOID"))
    {
        int fromId = js["FROMID"].get<int>();
        int toId = js["TOID"].get<int>();
        _friendModel.deleteFriend(fromId, toId);
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::NORMAL_ACK)(
            conn, js, time);
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

void FriendService::allFriends(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID"))
    {
        int fromId = js["FROMID"].get<int>();
        std::vector<User> friendUsers = _friendModel.query(fromId);
        std::vector<Group> friendGroups = _groupModel.queryGroups(fromId);

        std::vector<std::unordered_map<std::string, std::string>> friends;
        for (auto &i : friendUsers)
        {
            std::unordered_map<std::string, std::string> obj;
            obj["id"] = std::to_string(i.id());
            obj["name"] = i.name();
            obj["isGroup"] = "0";
            friends.push_back(obj);
        }
        for (auto &i : friendGroups)
        {
            std::unordered_map<std::string, std::string> obj;
            obj["id"] = std::to_string(i.id());
            obj["name"] = i.name();
            obj["isGroup"] = "1";
            friends.push_back(obj);
        }
        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::FRIEND_QUERY);
        response["errno"] = 200;
        response["friends"] = friends;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}
bool isConvertibleToNumber(const std::string &str)
{
    for (char c : str)
    {
        if (!std::isdigit(c))
        {
            return false; // Found a non-digit character
        }
    }

    return true; // All characters are digits
}
void FriendService::searchFriends(const muduo::net::TcpConnectionPtr &conn,
                                  nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("keyword"))
    {
        std::vector<std::unordered_map<std::string, std::string>> results;
        std::string keyword = js["keyword"].get<std::string>();

        if (isConvertibleToNumber(keyword))
        {
            int key = std::stoi(keyword);

            User u = _userModel.query(key);
            if (u.id() != -1)
            {
                std::unordered_map<std::string, std::string> obj;
                obj["id"] = std::to_string(u.id());
                obj["name"] = u.name();
                obj["isGroup"] = "0";
                results.push_back(obj);
            }

            Group g = _groupModel.queryGroupById(key);
            if (g.id() != -1)
            {
                std::unordered_map<std::string, std::string> obj;
                obj["id"] = std::to_string(g.id());
                obj["name"] = g.name();
                obj["isGroup"] = "1";
                obj["desc"] = g.desc();
                results.push_back(obj);
            }
        }
        std::vector<Group> gs = _groupModel.queryGroups(keyword);
        for (auto &g : gs)
        {
            std::unordered_map<std::string, std::string> obj;
            obj["id"] = std::to_string(g.id());
            obj["name"] = g.name();
            obj["isGroup"] = "1";
            obj["desc"] = g.desc();
            results.push_back(obj);
        }

        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::ACCOUNT_QUERY);
        response["errno"] = 200;
        response["results"] = results;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

FriendService &FriendService::instance()
{
    static FriendService friendService;
    return friendService;
}
FriendService::FriendService()
{
    // do nothing
}
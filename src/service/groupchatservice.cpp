#include "groupchatservice.h"

#include <string>
#include <vector>

#include "connectionhandler.h"
#include "msgdispatcher.h"
#include "redisconnectionpool.h"

void GroupChatService::createGroup(const muduo::net::TcpConnectionPtr &conn,
                                   nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID"))
    {
        int fromId = js["FROMID"].get<int>();
        std::string name = js["name"].get<std::string>();
        std::string description = js["description"].get<std::string>();
        Group g{-1, name, description};
        _groupModel.createGroup(g);
        _groupModel.addGroup(fromId, g.id(), "creator");

        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::GROUP_CREATE);
        response["errno"] = 200;
        response["GROUPID"] = g.id();
        response["name"] = name;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

// 给出群名称模糊搜索群聊
void GroupChatService::groupQuery(const muduo::net::TcpConnectionPtr &conn,
                                  nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("name"))
    {
        std::string name = js["name"].get<std::string>();
        std::vector<Group> groups = _groupModel.queryGroups(name);
        std::vector<nlohmann::json> groupsV;
        for (auto &i : groups)
        {
            nlohmann::json obj; // 一个群对象
            obj["id"] = std::to_string(i.id());
            obj["name"] = i.name();
            obj["desc"] = i.desc();

            std::vector<GroupUser> gUsers = i.users();
            std::vector<std::unordered_map<std::string, std::string>> gUsersV;
            for (auto &u : gUsers)
            {
                std::unordered_map<std::string, std::string> uV;
                uV["id"] = std::to_string(u.id());
                uV["name"] = u.name();
                uV["state"] = u.state();
                gUsersV.push_back(uV);
            }
            obj["gUsers"] = gUsersV;
            groupsV.push_back(obj);
        }

        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
        response["errno"] = 200;
        response["groups"] = groupsV;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

void GroupChatService::addGroup(const muduo::net::TcpConnectionPtr &conn,
                                nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID") && js.contains("GROUPID"))
    {
        int fromId = js["FROMID"].get<int>();
        int groupId = js["GROUPID"].get<int>();

        _groupModel.addGroup(fromId, groupId, "normal");
        std::string name = _groupModel.queryGroupById(groupId).name();
        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::GROUP_ADD);
        response["errno"] = 200;
        response["groupId"] = groupId;
        response["name"] = name;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

void GroupChatService::groupChat(const muduo::net::TcpConnectionPtr &conn,
                                 nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID") && js.contains("GROUPID"))
    {
        int groupId = js["GROUPID"].get<int>();
        int fromId = js["FROMID"].get<int>();
        std::vector<int> uIds = _groupModel.queryGroupUsers(groupId);
        for (auto toUserId : uIds)
        {
            if (fromId == toUserId)
                continue;
            if (!ConnectionHandler::instance().forwardMsg(toUserId,
                                                          js.dump()))
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
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

// 根据用户id查找这个用户的所有加入的群聊
void GroupChatService::groups(const muduo::net::TcpConnectionPtr &conn,
                              nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("FROMID"))
    {
        int fromId = js["FROMID"].get<int>();
        std::vector<Group> groups = _groupModel.queryGroups(fromId);
        std::vector<nlohmann::json> groupsV;
        for (auto &i : groups)
        {
            nlohmann::json obj; // 一个群对象
            obj["id"] = std::to_string(i.id());
            obj["name"] = i.name();
            obj["desc"] = i.desc();

            std::vector<GroupUser> gUsers = i.users();
            std::vector<std::unordered_map<std::string, std::string>> gUsersV;
            for (auto &u : gUsers)
            {
                std::unordered_map<std::string, std::string> uV;
                uV["id"] = std::to_string(u.id());
                uV["name"] = u.name();
                uV["state"] = u.state();
                gUsersV.push_back(uV);
            }
            obj["gUsers"] = gUsersV;
            groupsV.push_back(obj);
        }

        nlohmann::json response;
        response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
        response["errno"] = 200;
        response["groups"] = groupsV;
        conn->send(response.dump());
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

GroupChatService &GroupChatService::instance()
{
    static GroupChatService groupChatService;
    return groupChatService;
}

GroupChatService::GroupChatService()
{
    // do nothing
}
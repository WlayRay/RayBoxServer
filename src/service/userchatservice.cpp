#include "userchatservice.h"

#include <string>

#include "connectionhandler.h"
#include "msgdispatcher.h"
#include "redisconnectionpool.h"

void UserChatService::userChat(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js, muduo::Timestamp time)
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

UserChatService &UserChatService::instance()
{
    static UserChatService userChatService;
    return userChatService;
}

UserChatService::UserChatService()
{
    // do nothing
}
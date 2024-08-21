#include "userservice.h"

#include <mutex>
#include <string>
#include <unordered_map>

#include "connectionhandler.h"
#include "msgdispatcher.h"
#include "msgtypeenum.h"
// 处理用户登录
void UserService::login(const muduo::net::TcpConnectionPtr &conn,
                        nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("USERID") && js.contains("password"))
    {
        int userId = js["USERID"];
        std::string password = js["password"];
        User userInDb = _userModel.query(userId);

        if (userId == userInDb.id() && password == userInDb.password())
        {
            // if ("online" == userInDb.state()) {
            //     // 重复登录
            //     nlohmann::json response;
            //     response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
            //     response["errno"] = 301;
            //     response["msg"] = "already logged in, use another account.";

            //     conn->send(response.dump());
            // } else {
            // 登录成功
            // 更新在线状态
            _userModel.updateState(userId, "online");

            // 保存TcpConnection
            ConnectionHandler::instance().addConnection(userId, conn);

            MsgDispatcher::instance().getMsgHandler(
                MsgTypeEnum::NORMAL_ACK)(conn, js, time);
            // }
        }
        else
        {
            // 登录失败
            nlohmann::json response;
            response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
            response["errno"] = 302;
            response["msg"] = "wrong id or wrong password, try again.";
            conn->send(response.dump());
        }
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

// 处理用户注册
void UserService::reg(const muduo::net::TcpConnectionPtr &conn,
                      nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("name") && js.contains("password"))
    {
        std::string name = js["name"];
        std::string password = js["password"];
        User user{-1, name, password};
        if (_userModel.insert(user))
        {
            // 注册成功
            nlohmann::json response;
            response["msgType"] = static_cast<int>(MsgTypeEnum::NORMAL_ACK);
            response["errno"] = 200;
            response["USERID"] = user.id();
            conn->send(response.dump());
        }
        else
        {
            // 注册失败
            MsgDispatcher::instance().getMsgHandler(
                MsgTypeEnum::SERVER_PROCESS_ERROR)(conn, js, time);
        }
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

// 用户正常退出
void UserService::logout(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js, muduo::Timestamp time)
{
    if (js.contains("USERID"))
    {
        int userId = js["USERID"];
        userId = ConnectionHandler::instance().delConnection(userId);
        if (userId != -1)
        {
            // 成功删除
            _userModel.updateState(userId, "offline");
            MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::NORMAL_ACK)(
                conn, js, time);
        }
        else
        {
            // 未知的userId
            MsgDispatcher::instance().getMsgHandler(
                MsgTypeEnum::PARSE_JSON_ERROR)(conn, js, time);
        }
    }
    else
    {
        // 不合法的json
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, time);
    }
}

// 客户端异常断开连接(由muduo回调，不是根据消息类型回调)
// TODO 考虑网络异常，用户已下线但连接依然存在的情况（解决办法：采用心跳机制）
void UserService::clientCloseException(
    const muduo::net::TcpConnectionPtr &conn)
{
    int userId = ConnectionHandler::instance().delConnection(conn);

    if (userId != -1)
    {
        _userModel.updateState(userId, "offline");
    }
}

// 服务器主动关闭时回调（响应CTRL+C事件，不是根据消息类型回调）
void UserService::logoutAll()
{
    _userModel.offlineAll();
}

// 单例，考虑线程安全
UserService &UserService::instance()
{
    static UserService userService;
    return userService;
}

UserService::UserService()
{
    // do nothing
}
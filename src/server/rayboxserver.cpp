#include "rayboxserver.h"

#include <functional>
#include <string>

#include "logger.h"
#include "msgdispatcher.h"
#include "msgtypeenum.h"
#include "redisconnectionpool.h"
#include "connectionhandler.h"
#include "userservice.h"
RayBoxServer::RayBoxServer(muduo::net::EventLoop *loop,
                           const muduo::net::InetAddress &listenAddr,
                           const std::string &name)
    : _server(loop, listenAddr, name), _loop(loop)
{
    _server.setConnectionCallback(
        std::bind(&RayBoxServer::onConnection, this, std::placeholders::_1));
    _server.setMessageCallback(
        std::bind(&RayBoxServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    _server.setThreadNum(4);

    // 订阅redis消息
    sw::redis::Subscriber &sub = RedisConnectionPool::instance().getSubscriber();
    sub.psubscribe("RayBox-user-*");
    // 设置回调函数
    sub.on_pmessage(
        [](std::string pattern, std::string channel, std::string msg)
        {
            std::string str = channel;
            std::string prefix = "RayBox-user-";
            int userId;

            if (str.substr(0, prefix.size()) == prefix)
            {
                userId = std::stoi(str.substr(prefix.size()));
                ConnectionHandler::instance().forwardMsg(userId, msg);
            }
            else
            {
                // 非相关通道下的消息
            }
        });
    // 开启单独的子线程消费
    std::thread t([&]()
                  {
        while (true) {
            try {
                sub.consume();
            } catch (const sw::redis::Error& err) {
                LOG_ERROR("%s | catched redis error, reason:%s", __func__,
                          err.what());
            }
        } });
    t.detach();
}

void RayBoxServer::start()
{
    _server.start();
}

void RayBoxServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        UserService::instance().clientCloseException(conn);
        conn->shutdown();
    }
}

void RayBoxServer::onMessage(const muduo::net::TcpConnectionPtr &conn,
                             muduo::net::Buffer *buffer,
                             muduo::Timestamp timestamp)
{
    // TODO
    // 对接收到的消息进行扩展，使其支持图片或者其它二进制文件类型，可以对首部字节进行判断分类处理
    std::string buf = buffer->retrieveAllAsString();
    LOG_INFO("%s | recv:%s", __func__, buf.c_str());

    nlohmann::json js;
    MsgHandler msghandler;
    try
    {
        js = nlohmann::json::parse(buf);
        MsgTypeEnum msgType = static_cast<MsgTypeEnum>(js["msgType"].get<int>());
        msghandler = MsgDispatcher::instance().getMsgHandler(msgType);
        msghandler(conn, js, timestamp);
    }
    catch (nlohmann::json::exception e)
    {
        MsgDispatcher::instance().getMsgHandler(MsgTypeEnum::PARSE_JSON_ERROR)(
            conn, js, timestamp);
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " ip port" << std::endl;
        return 1;
    }

    std::string ip = argv[1];
    std::string port = argv[2];

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(ip, std::stoi(port));
    RayBoxServer server(&loop, addr, "RAYBOX-SERVER");

    server.start();
    loop.loop();
    return 0;
}
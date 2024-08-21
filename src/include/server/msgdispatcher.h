#pragma once
#include <muduo/net/TcpConnection.h>

#include <functional>
#include <json.hpp>
#include <unordered_map>

#include "msgtypeenum.h"

using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &,
                                      nlohmann::json &js, muduo::Timestamp)>;
// 将消息按照类型分发给对应的回调函数
class MsgDispatcher
{
public:
    static MsgDispatcher &instance();
    MsgHandler getMsgHandler(MsgTypeEnum msgTypeEnum);
    MsgDispatcher(const MsgDispatcher &) = delete;
    MsgDispatcher(MsgDispatcher &&) = delete;

private:
    // 单例，考虑线程安全
    MsgDispatcher();
    // 在构造时已经注册好了所有回调函数，后续获取handler也是只读不写，故线程安全
    std::unordered_map<MsgTypeEnum, MsgHandler> _msgHandlerMap;
};
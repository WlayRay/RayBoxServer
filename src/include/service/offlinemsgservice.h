#pragma once

#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>

#include <json.hpp>

#include "offlinemessagemodel.h"

// 获取离线消息
class OfflineMsgService {
public:
    void getAllOfflineMsgs(const muduo::net::TcpConnectionPtr &conn,
                           nlohmann::json &js, muduo::Timestamp time);

    static OfflineMsgService &instance();
    OfflineMsgService(const OfflineMsgService &) = delete;
    OfflineMsgService(OfflineMsgService &&) = delete;

private:
    OfflineMsgService() = default;
    OfflineMsgModel _offlineMsgModel;
};
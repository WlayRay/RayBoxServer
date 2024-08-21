#include "offlinemsgservice.h"
#include "msgtypeenum.h"
void OfflineMsgService::getAllOfflineMsgs(
    const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js,
    muduo::Timestamp time)
{
    int fromId = js["FROMID"].get<int>();
    std::vector<std::string> rs = _offlineMsgModel.query(fromId);
    _offlineMsgModel.remove(fromId);
    nlohmann::json resp;
    resp["msgType"] = static_cast<int>(MsgTypeEnum::OFFLINE_MSG_QUERY);
    resp["errno"] = 200;
    resp["msgs"] = rs;
    conn->send(resp.dump());
}

OfflineMsgService &OfflineMsgService::instance()
{
    static OfflineMsgService oms;
    return oms;
}

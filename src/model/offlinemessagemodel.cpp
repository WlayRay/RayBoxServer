#include "offlinemessagemodel.h"

#include <cppconn/exception.h>

#include <string>
#include <vector>

#include "logger.h"
#include "mysqlconnectionpool.h"

bool OfflineMsgModel::insert(int userId, std::string msg)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into OfflineMessage(userid, message) values(%d, '%s')",
             userId, msg.c_str());
    LOG_INFO("%s | %s", __func__, sql);

    try
    {
        return MysqlConnectionPool::getInstance().getConnection()->update(sql);
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return false;
    }
}

bool OfflineMsgModel::remove(int userId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "delete from OfflineMessage where userid=%d",
             userId);
    LOG_INFO("%s | %s", __func__, sql);

    try
    {
        return MysqlConnectionPool::getInstance().getConnection()->update(sql);
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return false;
    }
}

std::vector<std::string> OfflineMsgModel::query(int userId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select message from OfflineMessage where userid = %d", userId);
    LOG_INFO("%s | %s", __func__, sql);

    std::vector<std::string> vec;
    std::shared_ptr<sql::ResultSet> rs;
    try
    {
        rs = MysqlConnectionPool::getInstance().getConnection()->query(sql);
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return vec;
    }
    while (rs->next())
    {
        vec.emplace_back(rs->getString("message"));
    }
    return vec;
}
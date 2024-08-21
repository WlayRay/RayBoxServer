#include "friendmodel.h"

#include <cppconn/exception.h>

#include "logger.h"
#include "mysqlconnectionpool.h"
bool FriendModel::insert(int userId, int friendId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into Friend(userid, friendid) values(%d, %d)", userId,
             friendId);
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

bool FriendModel::deleteFriend(int userId, int friendId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "delete from Friend where userid=%d and friendid=%d", userId,
             friendId);
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

std::vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};

    snprintf(sql, sizeof(sql),
             "select a.id, a.name, a.state from User a inner join Friend b on "
             "b.friendid = a.id where b.userid=%d",
             userId);
    LOG_INFO("%s | %s", __func__, sql);
    std::vector<User> vec;

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
        vec.emplace_back(User(rs->getInt("id"), rs->getString("name"), "",
                              rs->getString("state")));
    }
    return vec;
}
#include "groupmodel.h"

#include <cppconn/exception.h>

#include <vector>

#include "logger.h"
#include "mysqlconnectionpool.h"
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
             group.name().c_str(), group.desc().c_str());
    LOG_INFO("%s | %s", __func__, sql);

    try
    {
        std::shared_ptr<MysqlConnection> conn =
            MysqlConnectionPool::getInstance().getConnection();
        bool flag = conn->update(sql);
        // use the same connection immediately to get the auto_increment column
        std::shared_ptr<sql::ResultSet> rs =
            conn->query("SELECT LAST_INSERT_ID()");
        while (rs->next())
        {
            group.setId(rs->getInt(1));
        }
        return flag;
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return false;
    }
}

bool GroupModel::addGroup(int userId, int groupId, std::string grouprole)
{
    char sql[1024] = {0};
    snprintf(
        sql, sizeof(sql),
        "insert into GroupUser(groupid, userid, grouprole) values(%d, %d, '%s')",
        groupId, userId, grouprole.c_str());
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

std::vector<Group> GroupModel::queryGroups(int userId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select a.id, a.groupname, a.groupdesc from AllGroup a inner join \
         GroupUser b on a.id = b.groupid where b.userid=%d",
             userId);
    LOG_INFO("%s | %s", __func__, sql);
    std::vector<Group> vec;
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
        vec.emplace_back(Group(rs->getInt("id"), rs->getString("groupname"),
                               rs->getString("groupdesc")));
    }
    return vec;
}

Group GroupModel::queryGroupById(int groupId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select a.id, a.groupname, a.groupdesc from AllGroup a where a.id = %d",
             groupId);
    LOG_INFO("%s | %s", __func__, sql);
    Group g;
    std::shared_ptr<sql::ResultSet> rs;
    try
    {
        rs = MysqlConnectionPool::getInstance().getConnection()->query(sql);
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return g;
    }

    while (rs->next())
    {
        g = Group(rs->getInt("id"), rs->getString("groupname"),
                  rs->getString("groupdesc"));
    }
    return g;
}

std::vector<Group> GroupModel::queryGroups(std::string groupname)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select a.id, a.groupname, a.groupdesc from AllGroup a where a.groupname "
             "like '%%%s%%'",
             groupname.c_str());
    LOG_INFO("%s | %s", __func__, sql);

    std::vector<Group> vec;
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
        vec.emplace_back(Group(rs->getInt("id"), rs->getString("groupname"),
                               rs->getString("groupdesc")));
    }
    return vec;
}

std::vector<int> GroupModel::queryGroupUsers(int groupId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select userid from GroupUser where groupid = %d", groupId);
    LOG_INFO("%s | %s", __func__, sql);

    std::vector<int> vec;
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
        vec.emplace_back(rs->getInt("userid"));
    }
    return vec;
}
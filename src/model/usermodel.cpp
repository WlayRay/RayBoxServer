#include "usermodel.h"

#include <cppconn/exception.h>

#include "logger.h"
#include "mysqlconnectionpool.h"
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "insert into User(name, password, state) values('%s', '%s', '%s')",
             user.name().c_str(), user.password().c_str(),
             user.state().c_str());
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
            user.setId(rs->getInt(1));
        }
        return flag;
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return false;
    }
}

User UserModel::query(int userId)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql),
             "select id, name, password, state from User where id = %d",
             userId);
    LOG_INFO("%s | %s", __func__, sql);

    User user;
    std::shared_ptr<sql::ResultSet> rs;
    try
    {
        rs = MysqlConnectionPool::getInstance().getConnection()->query(sql);
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return user;
    }
    while (rs->next())
    {
        user = User(rs->getInt("id"), rs->getString("name"),
                    rs->getString("password"), rs->getString("state"));
    }
    return user;
}

bool UserModel::updateState(int userId, std::string state)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "update User set state = '%s' where id = %d",
             state.c_str(), userId);
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

bool UserModel::offlineAll()
{
    try
    {
        return MysqlConnectionPool::getInstance().getConnection()->update(
            "update User set state='offline'");
    }
    catch (sql::SQLException e)
    {
        LOG_ERROR("%s | %s", __func__, e.what());
        return false;
    }
}
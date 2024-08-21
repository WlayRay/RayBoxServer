#include "mysqlconnection.h"

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <cstdio>
#include <memory>

MysqlConnection::MysqlConnection(const std::string &ip, unsigned short port,
                                 const std::string &user,
                                 const std::string &password,
                                 const std::string &dbname,
                                 const std::string &charset)
{
    sql::Driver *driver = get_driver_instance(); // no need to delete

    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "tcp://%s:%d", ip.c_str(), port);

    _conn = driver->connect(buffer, user, password);
    _conn->setSchema(dbname);
    _stmt = _conn->createStatement();

    // 保证中文正常显示
    snprintf(buffer, sizeof(buffer), "set names %s", charset.c_str());
    _stmt->execute(buffer);
}

bool MysqlConnection::update(std::string sql)
{
    return _stmt->executeUpdate(sql);
}

std::shared_ptr<sql::ResultSet> MysqlConnection::query(std::string sql)
{
    return std::shared_ptr<sql::ResultSet>(
        _stmt->executeQuery(sql)); // rvalue optimization
}

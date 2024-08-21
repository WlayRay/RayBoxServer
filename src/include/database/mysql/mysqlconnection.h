#pragma once
#include <bits/types/clock_t.h>
#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>

#include <memory>
#include <string>

// mysql 数据库连接包装类， 基于RAII构造
class MysqlConnection {
public:
    // 初始化数据库连接
    MysqlConnection(const std::string& ip, unsigned short port,
                    const std::string& user, const std::string& password,
                    const std::string& dbname,
                    const std::string& charset);

    // 释放数据库连接资源
    ~MysqlConnection() {
        delete _conn;
        delete _stmt;
    }

    // 禁用拷贝
    MysqlConnection(const MysqlConnection&) = delete;
    MysqlConnection& operator=(const MysqlConnection&) = delete;
    // 禁用移动
    MysqlConnection(MysqlConnection&&) = delete;
    MysqlConnection& operator=(MysqlConnection&&) = delete;

    // 更新操作 insert delete update
    bool update(std::string sql);

    // 查询操作 select
    std::shared_ptr<sql::ResultSet> query(std::string sql);

    // 获取sql::PreparedStatement 对象
    std::unique_ptr<sql::PreparedStatement> pstmt(std::string sql) {
        return std::unique_ptr<sql::PreparedStatement>(
            _conn->prepareStatement(sql));
    }

    // 获取空闲时间
    clock_t getIdleTime() const {
        return clock() - _idleStartPoint;
    }

    // 刷新开始空闲时间点
    void refreshIdleStartPoint() {
        _idleStartPoint = clock();
    }

private:
    // 表示和MySQL Server的一条连接
    sql::Connection* _conn;
    sql::Statement* _stmt;

    // 记录空闲状态起始时间点
    clock_t _idleStartPoint;
};

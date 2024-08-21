#include "mysqlconnectionpool.h"

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "logger.h"
#include "mysqlconnection.h"

MysqlConnectionPool &MysqlConnectionPool::getInstance(
    const std::string &configFilePath)
{
    static MysqlConnectionPool pool(configFilePath);
    return pool;
}

static std::unordered_map<std::string, std::string> ReadConfigFile(
    const std::string &filePath);

bool MysqlConnectionPool::loadConfigFile(std::string configFilePath)
{
    std::unordered_map<std::string, std::string> configMap =
        ReadConfigFile(configFilePath);

    const char *keys[] = {"ip", "port", "username",
                          "password", "dbname", "charset"};
    for (auto k : keys)
    {
        if ("" == configMap[k])
        {
            LOG_ERROR("%s is empty", k);
            // todo 考虑是否直接退出
            return false;
        }
    }

    _ip = configMap["ip"];
    _port = std::stoi(configMap["port"]);
    _username = configMap["username"];
    _password = configMap["password"];
    _dbname = configMap["dbname"];
    _charset = configMap["charset"];

    // the following configurations can use the defaults
    _initSize =
        "" == configMap["initSize"] ? 10 : std::stoi(configMap["initSize"]);
    _maxSize =
        "" == configMap["maxSize"] ? 1024 : std::stoi(configMap["maxSize"]);
    _maxIdleTime = "" == configMap["maxIdleTime"]
                       ? 60
                       : std::stoi(configMap["maxIdleTime"]);
    _connectionTimeout = "" == configMap["connectionTimeout"]
                             ? 100
                             : std::stoi(configMap["connectionTimeout"]);
    return true;
}

std::unordered_map<std::string, std::string> ReadConfigFile(
    const std::string &filePath)
{
    std::unordered_map<std::string, std::string> configMap;
    std::ifstream inputFile(filePath);
    std::string line;

    if (inputFile.is_open())
    {
        while (std::getline(inputFile, line))
        {
            // Skip empty lines or lines starting with '#'
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            // Find the position of the '=' character
            size_t equalPos = line.find('=');
            if (equalPos != std::string::npos)
            {
                // Extract the key and value
                std::string key = line.substr(0, equalPos);
                std::string value = line.substr(equalPos + 1);

                // Remove leading and trailing whitespaces from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                // Add the key-value pair to the map
                configMap[key] = value;
            }
        }
        inputFile.close();
    }
    else
    {
        LOG_ERROR("Failed to open mysql connection configuration file: %s",
                  filePath.c_str());
    }

    return configMap;
}

MysqlConnectionPool::MysqlConnectionPool(const std::string &configFilePath)
{
    if (!loadConfigFile(configFilePath))
    {
        LOG_ERROR("Failed to construct connection pool.");
        return;
    }

    // 创建初始连接
    for (int i = 0; i < _initSize; ++i)
    {
        MysqlConnection *conn = new MysqlConnection(
            _ip, _port, _username, _password, _dbname, _charset);
        conn->refreshIdleStartPoint(); // 刷新起始空闲时间点
        _connectionQueue.push(conn);
    }

    // 启动一个新的线程， 作为连接的生产者
    std::thread producer(
        std::bind(&MysqlConnectionPool::produceConnectionTask, this));
    producer.detach(); // 分离线程

    // 启动一个新的定时线程
    // 扫描连接池中的所有连接，释放超出最长空闲时间的连接
    std::thread releaseThread(
        std::bind(&MysqlConnectionPool::releaseIdleConnection, this));
    releaseThread.detach(); // 分离线程

    // 启动一个新的线程，用于定时发送心跳消息
    std::thread keepAliveThread(std::bind(&MysqlConnectionPool::keepAlive, this));
    keepAliveThread.detach(); // 分离线程
}

void MysqlConnectionPool::produceConnectionTask()
{ // 生产者生产连接
    for (;;)
    {
        std::unique_lock<std::mutex> unique_lock(_queueMtx);
        _cv.wait(unique_lock, [&]() -> bool
                 { return _connectionQueue.empty(); });

        if (_connectionQueue.size() < _maxSize)
        {
            MysqlConnection *conn = new MysqlConnection(
                _ip, _port, _username, _password, _dbname, _charset);

            conn->refreshIdleStartPoint(); // 刷新起始空闲时间点
            _connectionQueue.push(conn);
        }

        _cv.notify_all(); // 通知消费者使用连接
    }
}

std::shared_ptr<MysqlConnection> MysqlConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> unique_lock(_queueMtx);

    if (!_cv.wait_for(unique_lock, std::chrono::milliseconds(_connectionTimeout),
                      [&]() -> bool
                      { return _connectionQueue.size() > 0; }))
    {
        // 超时唤醒，仍然没有连接，不再等待直接返回
        return nullptr;
    };

    // 自定义删除器，将连接归还到队列中
    std::shared_ptr<MysqlConnection> conn(
        _connectionQueue.front(), [&](MysqlConnection *pconn) -> void
        {
            // 必须考虑线程安全
            std::unique_lock<std::mutex> lock(_queueMtx);
            pconn->refreshIdleStartPoint();
            _connectionQueue.push(pconn); });
    _connectionQueue.pop();
    _cv.notify_all();

    return conn;
}

void MysqlConnectionPool::releaseIdleConnection()
{
    for (;;)
    {
        // 定时检查
        std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        std::unique_lock<std::mutex> lock(_queueMtx);
        while (_connectionQueue.size() > _initSize)
        {
            MysqlConnection *p = _connectionQueue.front();
            if (p->getIdleTime() >= (_maxIdleTime * CLOCKS_PER_SEC))
            {
                _connectionQueue.pop();
                delete p;
            }
            else
            {
                // 队列头部的连接没有超出最长空闲，则其余连接均不会超过
                break;
            }
        }
    }
}

void MysqlConnectionPool::keepAlive()
{
    for (;;)
    {
        // 每分钟，发送一次心跳消息，实际就是执行"select 1"操作
        std::this_thread::sleep_for(std::chrono::seconds(60));
        std::unique_lock<std::mutex> lock(_queueMtx);
        for (int i = 0; i < _connectionQueue.size(); ++i)
        {
            MysqlConnection *p = _connectionQueue.front();
            p->query("select 1"); // 发送心跳
            _connectionQueue.push(p);
            _connectionQueue.pop();
        }
    }
}

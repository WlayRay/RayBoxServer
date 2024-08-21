#include "redisconnectionpool.h"

#include <sw/redis++/redis.h>

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include "connectionhandler.h"
#include "logger.h"

static std::unordered_map<std::string, std::string> ReadConfigFile(
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
        LOG_ERROR("Failed to open redis connection configuration file: %s",
                  filePath.c_str());
    }

    return configMap;
}
void RedisConnectionPool::publish(std::string channel, std::string msg)
{
    _redis.publish(channel, msg);
}

sw::redis::Subscriber &RedisConnectionPool::getSubscriber()
{
    static sw::redis::Subscriber sub = _redis.subscriber();
    return sub;
}

RedisConnectionPool &RedisConnectionPool::instance()
{
    static RedisConnectionPool redisConnectionPool("redis.cnf");
    return redisConnectionPool;
}

bool RedisConnectionPool::loadConfigFile(std::string configFilePath)
{
    std::unordered_map<std::string, std::string> configMap =
        ReadConfigFile(configFilePath);
    const char *keys[] = {"ip", "port"};
    for (auto k : keys)
    {
        if ("" == configMap[k])
        {
            LOG_ERROR("%s is empty", k);
            return false;
        }
    }
    _ip = configMap["ip"];
    _port = std::stoi(configMap["port"]);
    _username = configMap["username"];
    _password = configMap["password"];
    _db = configMap["db"];

    _size = "" == configMap["size"] ? 2 : std::stoi(configMap["size"]);
    _connection_idle_time = "" == configMap["connection_idle_time"]
                                ? 30000
                                : std::stoi(configMap["connection_idle_time"]);
    _wait_timeout = "" == configMap["wait_timeout"]
                        ? 100
                        : std::stoi(configMap["wait_timeout"]);
    return true;
}

RedisConnectionPool::RedisConnectionPool(const std::string &configFilePath)
{
    if (!loadConfigFile(configFilePath))
    {
        LOG_ERROR(
            "Failed to load redis configurations, rollback to default "
            "connection settings, which is tcp://127.0.0.1:6379.");
        // do nothing
        return;
    }

    sw::redis::ConnectionOptions connection_options;
    connection_options.host = _ip;
    connection_options.port = _port;
    if ("" != _username)
        connection_options.user = _username;
    if ("" != _password)
        connection_options.password = _password;
    if ("" != _db)
        connection_options.db = std::stoi(_db);
        
    sw::redis::ConnectionPoolOptions pool_options;
    pool_options.size = _size;
    pool_options.connection_idle_time = std::chrono::milliseconds(_connection_idle_time);
    pool_options.wait_timeout = std::chrono::milliseconds(_wait_timeout);

    _redis = sw::redis::Redis(connection_options, pool_options);

    // sw::redis::Subscriber sub = _redis.subscriber();
    // sub.psubscribe("raybox-user-*");
    // sub.on_pmessage(
    //     [](std::string pattern, std::string channel, std::string msg) {
    //         std::string str = channel;
    //         std::string prefix = "raybox-user-";
    //         int userId;

    //         if (str.substr(0, prefix.size()) == prefix) {
    //             userId = std::stoi(str.substr(prefix.size()));
    //             ConnectionHandler::instance().forwardMsg(userId, msg);
    //         } else {
    //             // 非相关通道下的消息
    //         }
    //     });
    // std::thread t([&]() {
    //     while (true) {
    //         try {
    //             sub.consume();
    //         } catch (const sw::redis::Error& err) {
    //             LOG_ERROR("%s | catched redis error, reason:%s", __func__,
    //                       err.what());
    //         }
    //     }
    // });
    // t.detach();
}

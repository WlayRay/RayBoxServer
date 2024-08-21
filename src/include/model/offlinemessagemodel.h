#pragma once
#include <string>
#include <vector>

class OfflineMsgModel
{
public:
    bool insert(int userId, std::string msg);
    bool remove(int userId);
    std::vector<std::string> query(int userId);
};
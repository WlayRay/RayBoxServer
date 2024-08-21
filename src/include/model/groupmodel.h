#pragma once
#include <string>
#include <vector>

#include "group.h"

class GroupModel
{
public:
    bool createGroup(Group &group);
    bool addGroup(int userId, int groupId, std::string role);
    std::vector<Group> queryGroups(int userId);
    Group queryGroupById(int groupId);
    std::vector<Group> queryGroups(std::string groupname);
    std::vector<int> queryGroupUsers(int groupId);
};

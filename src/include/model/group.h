#pragma once
#include <string>
#include <vector>

#include "groupuser.h"

class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
        : _id(id), _name(name), _desc(desc)
    {
    }

    void setId(int id)
    {
        this->_id = id;
    }
    void setName(std::string name)
    {
        this->_name = name;
    }
    void setDesc(std::string desc)
    {
        this->_desc = desc;
    }

    int id() const
    {
        return this->_id;
    }
    std::string name() const
    {
        return this->_name;
    }
    std::string desc() const
    {
        return this->_desc;
    }
    std::vector<GroupUser> &users()
    {
        return this->_users;
    }

private:
    int _id;
    std::string _name;
    std::string _desc;
    std::vector<GroupUser> _users;
};

static std::ostream &operator<<(std::ostream &os, const Group &val)
{
    os << "Group:" << val.id() << ", " << val.name() << ", " << val.desc();
    return os;
}
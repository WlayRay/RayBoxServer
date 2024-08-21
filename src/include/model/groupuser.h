#pragma once
#include <string>

#include "user.h"

class GroupUser : public User
{
public:
    GroupUser(int id = -1, std::string name = "", std::string password = "",
              std::string state = "offline", std::string role = "normal")
        : User(id, name, password, state), _role(role)
    {
    }
    void setRole(std::string role)
    {
        this->_role = role;
    }
    std::string role() const
    {
        return this->_role;
    }

private:
    std::string _role;
};
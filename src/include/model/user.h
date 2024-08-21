#pragma once
#include <iostream>
#include <ostream>
#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string password = "",
         std::string state = "offline")
        : _id(id), _name(name), _password(password), _state(state)
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
    void setPassword(std::string password)
    {
        this->_password = password;
    }
    void setState(std::string state)
    {
        this->_state = state;
    }

    int id() const
    {
        return this->_id;
    }
    std::string name() const
    {
        return this->_name;
    }
    std::string password() const
    {
        return this->_password;
    }
    std::string state() const
    {
        return this->_state;
    }

protected:
    int _id;
    std::string _name;
    std::string _password;
    std::string _state;
};

static std::ostream &operator<<(std::ostream &os, const User &val)
{
    os << "User:" << val.id() << ", " << val.name() << ", " << val.password()
       << ", " << val.state();
    return os;
}
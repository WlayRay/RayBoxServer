#pragma once
#include "user.h"
class UserModel
{
public:
    bool insert(User &user);

    User query(int userId);

    bool updateState(int userId, std::string state);

    bool offlineAll();
};
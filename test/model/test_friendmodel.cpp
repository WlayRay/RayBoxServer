#include <iostream>

#include "friendmodel.h"
int main() {
    FriendModel fm;
    if (fm.insert(5, 6)) {
        std::cout << "插入好友关系成功" << std::endl;
    }

    std::cout << "=====================" << std::endl;
    std::vector<User> us = fm.query(5);
    std::cout << "用户5有以下好友:" << std::endl;
    for (auto &i : us) {
        std::cout << i << std::endl;
    }
}
#include <iostream>
#include <vector>

#include "group.h"
#include "groupmodel.h"
int main() {
    Group g(1, "modern cpp", "everything about modern cpp");
    GroupModel gm;
    if (gm.createGroup(g)) {
        std::cout << "创建群成功" << std::endl;
    }

    std::cout << "======================" << std::endl;
    std::vector<Group> gs = gm.queryGroups("cpp");
    std::cout << "查询关键字【cpp】得到的群:" << std::endl;
    for (auto& i : gs) {
        std::cout << i << std::endl;
    }

    std::cout << "======================" << std::endl;
    if (gm.addGroup(5, 1, "normal")) {
        std::cout << "用户5加入群1成功" << std::endl;
    }

    std::cout << "======================" << std::endl;
    std::vector<Group> gs2 = gm.queryGroups(5);
    std::cout << "用户5加入的群:" << std::endl;
    for (auto& i : gs2) {
        std::cout << i << std::endl;
    }

    std::cout << "======================" << std::endl;
    std::vector<int> us = gm.queryGroupUsers(1);
    std::cout << "群1的所有用户:" << std::endl;
    for (auto& i : us) {
        std::cout << i << std::endl;
    }
}
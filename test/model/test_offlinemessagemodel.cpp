#include <iostream>
#include <vector>

#include "offlinemessagemodel.h"
int main() {
    OfflineMsgModel offmsgmodel;
    if (offmsgmodel.insert(5, "给用户5的离线消息内容")) {
        std::cout << "插入离线消息成功" << std::endl;
    }
    
    std::cout << "======================" << std::endl;
    std::vector<std::string> msgs = offmsgmodel.query(5);
    std::cout << "查询得到用户5的以下离线消息:" << std::endl;
    for (auto& s : msgs) {
        std::cout << s << std::endl;
    }

    std::cout << "======================" << std::endl;
    if (offmsgmodel.remove(5)) {
        std::cout << "删除用户5离线消息成功" << std::endl;
    }
}

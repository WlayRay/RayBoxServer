#pragma once

enum class MsgTypeEnum
{
    // 请求处理
    //-----用户------
    USER_LOGIN = 1,  // 用户登录请求
    USER_REG = 2,    // 用户注册请求
    USER_LOGOUT = 3, // 用户注销请求

    //-----聊天------
    USER_CHAT = 4, // 用户聊天消息

    //-----好友------
    FRIEND_REQUEST = 5, // 请求添加好友
    FRIEND_Add = 6,     // 添加好友关系
    FRIEND_DELETE = 7,  // 删除好友关系
    FRIEND_QUERY = 8,   // 查询好友关系

    //-----群聊------
    GROUP_CHAT = 9,           // 群聊消息
    GROUP_ADD = 10,           // 加群
    GROUP_QUERY = 11,         // 查询所有群
    GROUP_CREATE = 12,        // 创建群
    GROUP_QUERY_BY_NAME = 13, // 根据群名查询

    //-----离线消息------
    OFFLINE_MSG_QUERY = 14, // 获取离线消息

    //-----查询账号------
    ACCOUNT_QUERY = 15, // 搜索用户或者群聊

    // 响应处理
    NORMAL_ACK = 200, // 常规响应

    // 错误处理
    PARSE_JSON_ERROR = 401,     // json 字符串解析错误
    METHOD_NOT_FOUND = 404,     // 未找到请求的回调函数
    SERVER_PROCESS_ERROR = 500, // 服务器出错
};
// 这是server和client公共的头文件，双方都可以使用这个头文件
#ifndef PUBLIC_H
#define PUBLIC_H

#include <openssl/aes.h>  
#include <string>
#include <vector>
#include<muduo/base/Atomic.h>

extern muduo::AtomicInt64 g_onlineUsers;  // 定义聊天人数

enum EnMsgType{
    LOGIN_MSG = 1,  //登录消息
    LOGIN_MSG_ACK,  //登录响应消息
    LOGINOUT_MSG, // 用户注销消息
    REG_MSG, // 注册消息
    REG_MSG_ACK,  //注册响应消息
    ONE_CHAT_MSG, //聊天消息
    ADD_FRIEND_MSG,  //添加好友消息

    CREATE_GROUP_MSG, //创建群组
    ADD_GROUP_MSG,  //加入群组
    GROUP_CHAT_MSG,   //群聊天
};

namespace ChatCrypto {
    std::string encryptMessage(const std::string& msg, const std::string& key);  // 只这行
    std::string decryptMessage(const std::string& encrypted, const std::string& key);  // 只这行
}

#endif
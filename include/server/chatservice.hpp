// 注意，这是service , 是聊天服务器的业务类， server是服务器
#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Atomic.h> // 包含Atomic头文件

#include<unordered_map>
#include<functional>

#include<mutex>

#include "offlinemessagemodel.hpp"

#include "usermodel.hpp"
#include "friendmodel.hpp"
#include "redis.hpp"
#include "groupmodel.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

//消息处理器， 本质是一个匿名函数， 存在的目的： 这个类是专门用于处理用户登录和发送消息时的业务
//业务由一个专门的类来处理，完成将业务和网络连接解耦的功能
//这个类最终会由chatserver.cpp使用
// 函数模板，MsgHandler可以接收任何形式的函数
using MsgHandler = std:: function<void(const TcpConnectionPtr &conn,json &js, Timestamp)>;



class ChatService{

public:

     // 获取单例对象的接口函数
     // 获取本身
     static ChatService* instance();

    // 处理登录业务
     void login(const TcpConnectionPtr &conn,json &js,Timestamp time);

     //处理注册业务
     void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);

     //获取消息对应的处理器
     MsgHandler getHandler(int msgid);

     //处理客户端异常退出
     void clientCloseException(const TcpConnectionPtr &conn);

     //实现一对一聊天业务
     void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);

     //服务器异常，重置方法
     void reset();

     //添加好友业务  msgid id friendid
     void addFriend(const TcpConnectionPtr &conn, json &js , Timestamp time);

     //添加群组业务
     void createGroup(const TcpConnectionPtr &conn, json &is, Timestamp time);

     //加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);


private:
    ChatService();

    //存储消息id和其对应业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;


    // 定义一个互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    //存储用户的的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //数据操作类
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;

};

#endif
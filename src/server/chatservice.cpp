
// 这个cpp是对业务的实现 和 网络的连接 进行解耦的关键

// 同时：这也是一种可以学习的思想，项目要插入一个新的模块或者功能，就创建一个独立的新类来实现，防止和原来的老代码混合在一起
// 这个新类，一般是 ： xxx.hpp + xxx.cpp 组合

#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include "redis.hpp"
#include <muduo/base/Logging.h>
#include <vector>
#include <memory>
#include <string>
#include<iostream>

#include <muduo/base/Atomic.h>  // 新加，全局计数功能

//全局计数功能
// #include <muduo/net/http/HttpServer.h>
// #include <muduo/net/http/HttpRequest.h>
// #include <muduo/net/http/HttpResponse.h>

#include <muduo/base/Logging.h>
#include <fstream>



using namespace muduo;
using namespace  std;




//一一实现chatservice.hpp头文件中的类
/**
 * @brief 获取ChatService单例实例
 * @return 返回ChatService单例对象的指针
 * 
 * @note 采用懒汉式单例模式，保证整个程序中只有一个ChatService实例
 */
ChatService* ChatService::instance(){

    // ！！！ 生成一个静态实例ChatService，方便随时取出来使用

    static ChatService service;
    return &service;

}

//完成解耦的核心代码！
//注册消息以及对应的Handler回调操作
/**
 * @brief ChatService构造函数，注册所有消息处理器并连接Redis
 * 
 * 完成网络模块与业务模块解耦的核心实现，通过消息映射表将各种消息类型
 * 与对应的业务处理函数进行绑定，并初始化Redis连接用于集群通信。
 */
ChatService::ChatService(){

    //下面是一种常用的设计思想
    //单例模式，貌似是这个

    //将处理某个业务的具体函数 和 某个 常量 联系起来
    //要调用这个函数，只需要知道这个常量名就可
    //这里是： login + func_login


    //这个操作称为 注册

   _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
   _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend,this,_1,_2,_3)});
   _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
   _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this,_1,_2,_3)});
   _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout,this,_1,_2,_3)});

   // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }

}

// 获取消息对应的处理器
/**
 * @brief 根据消息ID获取对应的消息处理器
 * @param msgid 消息标识符
 * @return 对应的消息处理函数，如果未找到则返回空操作处理器
 * 
 * @note 如果消息ID不存在于映射表中，会记录错误日志并返回默认处理器
 */
MsgHandler ChatService::getHandler(int msgid){

    //记录错误日志,msgid获取业务函数失败

    // 根据用户发送的业务ID序号
    // 使用map获取对应的业务
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        
        //返回一个空的处理器，表示获取失败，空操作
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp){
            (void)conn;  
            (void)js;    
            LOG_ERROR <<"msgid"<<msgid<<"can not find handler!";
        };
    }
    else{
        return _msgHandlerMap[msgid];
    }

}

//处理登录业务
/**
 * @brief 处理用户登录业务
 * @param conn TCP连接对象
 * @param js 包含登录信息的JSON对象
 * @param time 时间戳
 * 
 * 验证用户身份信息，更新用户状态为在线，记录用户连接，并返回好友列表、
 * 群组信息和离线消息等数据。支持SHA256密码加密验证。
 */
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time){

    (void)conn;  // 避免警告
    (void)time;  

    int id = js["id"].get<int>();
    string pwd = js["password"].get<string>();  //不加getstring会导致转换错误

    LOG_INFO << "登录时输入的密码： " << pwd ;


    //这里又发生了一次serPwd？
    // 有问题的是query，没有将查询到的密码正确的转换出来

    User user = _userModel.query(id);  



    //SSL预防SQL注入
    // unsigned char hash[SHA256_DIGEST_LENGTH];
    // SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);
    // std::string hashedInput;

    // LOG_INFO << "输入哈希: " << hashedInput << " DB哈希: " << user.getPwd();  // 加这
    
    // for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    //     char buf[3];
    //     sprintf(buf, "%02x", hash[i]);
    //     hashedInput += buf;
    // }

    //
    LOG_INFO << "chatsevice的用户id为: "<< user.getId();

    LOG_INFO << "chatsevice的用户密码为: "<< user.getPwd();

    if(user.getId() == id && user.getPwd() != ""){

        //SSL预防SQL注入
        // 原加密
        // unsigned char hash[SHA256_DIGEST_LENGTH];
        // SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);
        // std::string hashedInput;
        // for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        //     char buf[3];
        //     sprintf(buf, "%02x", hash[i]);
        //     hashedInput += buf;
        // }
        // LOG_INFO << "输入哈希: " << hashedInput << " DB哈希: " << user.getPwd();


        // 登录哈希部分
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);

        // Base64编码
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string hashedInput(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);

        LOG_INFO << "输入时的哈希值: " << hashedInput << " 查询数据库里面存的哈希值: " << user.getPwd();

        if(user.getPwd() == hashedInput){
            LOG_INFO << "登录成功!";
            
            
            if(user.getState() == "online"){
                //用户已经登录，不允许重复登录
                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 2;
                response["errmsg"] = "this account is using, input another!";
                conn->send(response.dump());
 
            }else{

                // 核心!!: 数据库加锁
                // 一个函数可能同时被多个线程执行
                // 因此为了保证安全，有些操作需要加锁
                // 用户的重复登录没有关心，不影响，不需要加锁
                // 但是为了用户可以接收到消息，需要记录用户的连接conn
                // 为了防止线程重复建立连接，这里需要给conn的记录过程加锁
                // 记录用户连接，有消息时就向用户发送消息

                //使用作用域
                {
                    lock_guard<mutex> lock(_connMutex);
                    _userConnMap.insert({id,conn});
                }

                // id用户登录成功后，向redis订阅channel(id)
                _redis.subscribe(id); 


                // 登录成功，更新用户状态信息 state.offline=>online
                user.setState("online");
                _userModel.updateState(user);
                //登录成功

                //登录成功后，在线人数+1

                g_onlineUsers.increment();

                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 0;
                response["id"] = user.getId();
                response["name"] = user.getName();


            
                // 查询该用户是否有离线消息
                vector<string> vec = _offlineMsgModel.query(id);
                if (!vec.empty())
                {
                    response["offlinemsg"] = vec;
                    // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                    _offlineMsgModel.remove(id);
                }


                //登录成功后，要查询并将用户的好友信息返回过去
                vector<User> userVec = _friendModel.query(id);
                if (!userVec.empty())
                {
                    vector<string> vec2;
                    for (User &user : userVec)
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        vec2.push_back(js.dump());
                    }
                    response["friends"] = vec2;
                }


            // 查询用户的群组信息
                vector<Group> groupuserVec = _groupModel.queryGroups(id);
                if (!groupuserVec.empty())
                {
                    // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                    vector<string> groupV;
                    for (Group &group : groupuserVec)
                    {
                        json grpjson;
                        grpjson["id"] = group.getId();
                        grpjson["groupname"] = group.getName();
                        grpjson["groupdesc"] = group.getDesc();
                        vector<string> userV;
                        for (GroupUser &user : group.getUsers())
                        {
                            json js;
                            js["id"] = user.getId();
                            js["name"] = user.getName();
                            js["state"] = user.getState();
                            js["role"] = user.getRole();
                            userV.push_back(js.dump());
                        }
                        grpjson["users"] = userV;
                        groupV.push_back(grpjson.dump());
                    }

                    response["groups"] = groupV;
                }

                cout<<7<<endl;

                conn->send(response.dump());
            }
        } else {
            //  该用户不存在/用户存在但是密码错误，登录失败
                json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 1;
                response["errmsg"] = "id or password is invalid!";
                conn->send(response.dump());
        }
        
    }
    else{
    //  该用户不存在/用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
    

}

//处理注册业务
/**
 * @brief 处理用户注册业务
 * @param conn TCP连接对象
 * @param js 包含注册信息的JSON对象
 * @param time 时间戳
 * 
 * 创建新用户账户并将信息存储到数据库，返回注册结果。
 */
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp time){
    (void)conn;  // 避免警告
    (void)time;  

    string name = js["name"];
    string pwd = js["password"].get<string>();

    LOG_INFO << "注册时输入的密码： " << pwd ;

    User user;
    user.setName(name);

    // 先SHA256哈希
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);

        LOG_INFO << "hash"<< hash;
        // Base64编码 (32字节 → 44字符)
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);  // 无换行
        BIO_write(bio, hash, SHA256_DIGEST_LENGTH);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string hashedPwd(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);

        LOG_INFO << "第一次转换时的哈希值: " << hashedPwd;

    user.setPwd(hashedPwd);  // 存哈希

    //哈希加密
    // unsigned char hash[SHA256_DIGEST_LENGTH];
    // SHA256(reinterpret_cast<const unsigned char*>(pwd.c_str()), pwd.length(), hash);
    // std::string hashedPwd;
    // for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    //     char buf[3];
    //     sprintf(buf, "%02x", hash[i]);
    //     hashedPwd += buf;
    // }
    // user.setPwd(hashedPwd);  // 存哈希

    bool state = _userModel.insert(user);

    // LOG_INFO << "注册状态: " << state << " 新ID: " << user.getId();  // 加这行日志

    // cout<<"ChatService::reg"<<state<<endl;


    if(state){
        //注册成功
        json response;
        response["msgid"]= REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
        conn->send(response.dump());
    }
    else{
        // 注册失败
        json response;
        response["msgid"]= REG_MSG_ACK;
        response["errno"]=1;
        conn->send(response.dump());
    }

}

//处理客户端异常退出
/**
 * @brief 处理客户端异常断开连接
 * @param conn 发生异常的TCP连接对象
 * 
 * 清理异常断开用户的连接信息，更新用户状态为离线，并取消Redis订阅。
 */
void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    (void)conn;  // 避免警告

    User user;
    //关闭连接的过程，也要加锁
    {
       lock_guard<mutex> lock(_connMutex);
        
       //迭代器循环，寻找用户的连接
       for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
       {
           if(it->second == conn){
               
              //删除表并获取用户信息
              user.setId(it->first);
              _userConnMap.erase(it);
              break;
           }
       }
       

    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId()); 


    //更新用户的状态
    if(user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }

}


//服务器异常，重置方法
/**
 * @brief 重置所有用户状态为离线
 * 
 * 在服务器异常重启时调用，确保所有用户状态被正确重置。
 */
void ChatService::reset(){

    //把用户状态重置为offline

    _userModel.resetState();


}

//添加好友业务 msgid id friendid
/**
 * @brief 处理添加好友业务
 * @param conn TCP连接对象
 * @param js 包含好友信息的JSON对象
 * @param time 时间戳
 */
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js , Timestamp time){

    (void)conn;  // 避免警告
    (void)time;  

    int userid = js["id"].get<int>();

    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid,friendid);

}


//群组业务
// 创建群组业务
/**
 * @brief 处理创建群组业务
 * @param conn TCP连接对象
 * @param js 包含群组信息的JSON对象
 * @param time 时间戳
 */
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){

    (void)conn;  // 避免警告
    (void)time;  

    int userid = js["id"].get<int>();
    string name =js["groupname"];
    string desc=js["groupdesc"];
    // 存储新创建的群组信息
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group)){
        // 存储群组创建人信息
        _groupModel.addGroup(userid,group.getId(),"creator");
    }

}

//加入群组业务
/**
 * @brief 处理加入群组业务
 * @param conn TCP连接对象
 * @param js 包含群组信息的JSON对象
 * @param time 时间戳
 */
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){

    (void)conn;  // 避免警告
    (void)time;  

    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");

}

// 群组聊天业务
/**
 * @brief 处理群组聊天业务
 * @param conn TCP连接对象
 * @param js 包含群聊信息的JSON对象
 * @param time 时间戳
 * 
 * 向群组内所有成员转发消息，支持消息加密和跨服务器消息传递。
 */
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){

    (void)conn;  // 避免警告
    (void)time;  

   int userid = js["id"].get<int>();
   int groupid = js["groupid"].get<int>();
   vector<int>useridVec=_groupModel.queryGroupUsers(userid, groupid);

   //加密
    std::string msg = js["msg"].get<std::string>();
    std::string encryptedMsg = ChatCrypto::encryptMessage(msg, "mysecretkey12345");  // 加密
    js["msg"] = encryptedMsg; 

   //使用连接类，一定要注意加锁操作，否则会出现操作重复的情况
   lock_guard<mutex> lock(_connMutex);
   for (int id :useridVec){

      auto it = _userConnMap.find(id);
      if( it != _userConnMap.end()){
          //转发群消息
        it->second->send(js.dump());
      }
      else{
        // 查询toid是否在线 
        // ！！！ 如果在线，并且是在另一台服务器上，那就只能通过redis来传递消息
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
      }

   }

}


// 处理注销业务
/**
 * @brief 处理用户注销业务
 * @param conn TCP连接对象
 * @param js 包含注销信息的JSON对象
 * @param time 时间戳
 * 
 * 清理用户连接信息，取消Redis订阅，更新用户状态为离线。
 */
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{

    (void)conn;  // 避免警告
    (void)time;  

    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    //
    g_onlineUsers.decrement();  // 在线人数-1

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

//  一对一聊天业务
/**
 * @brief 处理一对一聊天业务
 * @param conn TCP连接对象
 * @param js 包含聊天信息的JSON对象
 * @param time 时间戳
 * 
 * 实现用户间一对一消息传递，支持消息加密、在线推送和离线存储。
 */
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{

    (void)conn;  // 避免警告
    (void)time;  
    int toid = js["toid"].get<int>();

    //进行加密
    std::string msg = js["msg"].get<std::string>();
    //  调用加密函数（例如AES加密）
    std::string encryptedMsg = ChatCrypto::encryptMessage(msg, "mysecretkey12345");  // 加密
    //  将加密后的内容放回json对象
    js["msg"] = encryptedMsg;  // 替换成加密的

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息   服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线 
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {

        // ！！！ 如果在线，并且是在另一台服务器上，那就只能通过redis来传递消息
        // ！！！ redis起到在集群服务器的中间人作用，负责传递消息 
        _redis.publish(toid, js.dump());
        return;
    }

    // toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 从redis消息队列中获取订阅的消息
/**
 * @brief 处理Redis订阅消息
 * @param userid 目标用户ID
 * @param msg 消息内容
 * 
 * 从Redis消息队列中获取订阅的消息并推送给相应用户。
 */
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    // 查询用户是否在线，如果在线就发送消息
    // 否则发送离线小i
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}
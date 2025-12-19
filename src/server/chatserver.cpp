#include "chatserver.hpp"
#include "json.hpp"
#include "public.hpp"

// 将某个要实现的新功能通过头文件的方式插入，称为：派发服务
// 这个头文件自动绑定了其cpp文件，所以只需要映入hpp就行了
#include "chatservice.hpp"

#include<string>
#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

/**
 * @brief ChatServer 构造函数，初始化聊天服务器并注册事件回调。
 * 
 * 该构造函数负责初始化 TCP 服务器实例，并设置连接建立/断开、消息到达等事件的核心回调函数。
 * 同时配置服务器内部处理线程的数量，为服务器启动做好准备[2,3](@ref)。
 * 
 * @param loop 事件循环指针，服务器将运行于此事件循环中。该参数是服务器运行的基础[7](@ref)。
 * @param listenAddr 服务器监听的网络地址，包含IP和端口号。服务器将在此地址上接受客户端连接[7](@ref)。
 * @param nameArg 聊天服务器的名称标识，可用于区分同一进程中的多个服务器实例[7](@ref)。
 * 
 * @note 构造函数中注册的回调函数（onConnection, onMessage）将在对应事件发生时被自动调用。
 */

ChatServer:: ChatServer(EventLoop *loop,const InetAddress& listenAddr, const string& nameArg)
            : _server(loop,listenAddr,nameArg), _loop(loop)
{
    
    //注册链接回调

    // 回调：某些事件发生时，会随之自动触发的函数
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数量
    _server.setThreadNum(4);

}

//启动服务

// ！！！ 启动后发生的事情
// 1. 创建四个子进程，对程序员和用户透明，程序员只能看到一个主进程，主进程只负责分配任务给四个子进程：  主进程：TcpServer+Eventloop，子进程：同样也是 TcpServer+Eventloop
// 2. Eventloop启动，开始监听指定的端口：如：127.0.0.1：6000 
// 3. 当端口发生事件，如：客户端申请连接./ChatClient 127.0.0.1：6000 ,  Eventloop开始调用Accept事件触发器，调用  预先绑定好的回调函数
// 4. 主进程触发回调函数，分配这个事情给下面的子进程处理
// 5. 子进程接收到任务， 子进程的Eventloop开始调用Accept事件触发器，执行回调函数，处理用户事件


void ChatServer::start(){
    _server.start();
}


//上报链接相关的回调函数
//当用户发生连接操作时，就会执行这个被绑定的操作
//server随时监控，只要发生连接相关操作就会执行

// ！！！绑定回调函数， 这里回调函数的具体操作被 解耦

/**
 * @brief 处理客户端连接状态变化的回调函数
 * 
 * 当客户端与服务器建立连接或断开连接时，muduo网络库会自动调用此函数。
 * 函数内部实现了连接超时管理机制：连接建立时启动心跳检测定时器，连接断开时清理资源。
 * 
 * @param conn TCP连接对象的智能指针，包含客户端的连接信息和方法[1,3](@ref)
 * 
 * @note 连接状态管理逻辑：
 * - 连接建立：启动30秒间隔的定时器发送心跳包，检测连接是否存活
 * - 连接断开：取消定时器，更新用户状态为离线，关闭网络连接[4,5](@ref)
 */
void ChatServer::onConnection(const TcpConnectionPtr &conn){
    
   // 返回用户的连接状态,如果不处于连接状态，要更新用户状态

   // 如果tcp连接符检测到不处于连接中，说明此处连接发生错误/连接中断，需要重置用户连接状态
   if(!conn->connected()){

      // 断开取消定时器
      EventLoop *loop = conn->getLoop();
      TimerId timerId = boost::any_cast<TimerId>(conn->getContext());
      loop->cancel(timerId);

      ChatService::instance()->clientCloseException(conn);  //更新用户状态
      conn->shutdown();       //关闭连接

   }else{
      
      // 设置定时器，超时断开踢掉
      conn->setContext(TimerId());  // 存定时器ID
        EventLoop *loop = conn->getLoop();
        TimerId timerId = loop->runEvery(30.0, [conn]() {  // 每30秒检查
            if (conn->connected()) {
                json js; js["msgid"] = 100;  // 心跳msgid，自定义
                conn->send(js.dump());
            }
        });
        conn->setContext(timerId);
   }

}

//上报读写事件相关信息的回调函数

/**
 * @brief 处理客户端消息到达的回调函数
 * 
 * 当服务器收到客户端发送的数据时自动调用此函数。负责消息解析、解密、心跳处理和业务派发。
 * 实现了网络模块与业务模块的解耦，通过消息ID将不同业务派发到对应的处理器[5,8](@ref)。
 * 
 * @param conn TCP连接对象，用于消息回复和连接管理
 * @param buffer 数据缓冲区，包含接收到的原始数据
 * @param time 消息到达的时间戳
 * 
 * @note 消息处理流程：
 * 1. 提取并解析JSON格式的消息
 * 2. 心跳消息处理：重置超时定时器
 * 3. 消息解密：对加密内容进行解密处理
 * 4. 业务派发：根据msgid找到对应的业务处理器执行具体逻辑[4,8](@ref)
 */
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                Buffer *buffer,
                Timestamp time)
{

   string buf = buffer->retrieveAllAsString();

   //获取用户输入的缓冲区数据
   json js = json::parse(buf);

   // 收到消息，重置定时器

   if (js["msgid"] == 100) {  // 心跳响应
        // 重置定时器：取消旧，加新
        EventLoop *loop = conn->getLoop();
        TimerId oldId = boost::any_cast<TimerId>(conn->getContext());
        loop->cancel(oldId);
        TimerId newId = loop->runAfter(60.0, [conn]() { conn->shutdown(); });  // 60秒无消息关连
        conn->setContext(newId);
    }

    //

   //解密
   if (js.contains("msg")) {
        std::string encrypted = js["msg"].get<std::string>();
        std::string msg = ChatCrypto::decryptMessage(encrypted, "mysecretkey12345");  // 解密
        js["msg"] = msg;  // 替换回明文
    }

   //接下来，为了实现业务功能和网络功能的分离，要进行解耦
   //也就是通过一个单独的函数来实现业务功能
   //不过在大型项目中，这个函数通过会通过创建一个新的类的方式来包装起来
   //通常是通过头文件的方式来包装，这样比较方便，称为：派发服务

   //instance会初始化并返回一个ChatService类对象，用于当作getHandler的参数传入
   auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());

   //js.get<> 是一个模板方法， 输入什么类型，json就会转换为对应的类型，比如get<int>

   //获得处理业务的函数处理器，来执行相应业务
   msgHandler(conn,js,time);

                    
}

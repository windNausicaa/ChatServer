#include "chatserver.hpp"
#include "json.hpp"

// 将某个要实现的新功能通过头文件的方式插入，称为：派发服务
// 这个头文件自动绑定了其cpp文件，所以只需要映入hpp就行了
#include "chatservice.hpp"

#include<string>
#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

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
void ChatServer::onConnection(const TcpConnectionPtr &conn){
    
   // 返回用户的连接状态,如果不处于连接状态，要更新用户状态

   // 如果tcp连接符检测到不处于连接中，说明此处连接发生错误/连接中断，需要重置用户连接状态
   if(!conn->connected()){

      ChatService::instance()->clientCloseException(conn);  //更新用户状态
      conn->shutdown();       //关闭连接

   }

}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                Buffer *buffer,
                Timestamp time)
{

   string buf = buffer->retrieveAllAsString();

   //获取用户输入的缓冲区数据
   json js = json::parse(buf);

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

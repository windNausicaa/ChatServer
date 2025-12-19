#include "chatserver.hpp"
#include<iostream>
#include "chatservice.hpp"
#include <signal.h>


#include <muduo/base/Atomic.h>  // 新加,统计在线人数
#include <muduo/base/Logging.h>
#include <fstream>

using namespace std;


muduo::AtomicInt64 g_onlineUsers;  //聊天人数


//服务器异常中止ctrl+C,执行重置函数重载online状态

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);  // 系统调用，结束时重置user的状态信息

    EventLoop loop;
    InetAddress addr(ip, port);   // muduo库函数
    ChatServer server(&loop, addr, "ChatServer");



    server.start();    // ！！！ 程序的开始  ，服务器本体

    //开始循环前，实时统计在线人数
    loop.runEvery(15.0, []() {
        std::ofstream file("/tmp/chat_metrics.txt");
        file << "online_users " << g_onlineUsers.get() << "\n";
        file.close();
        LOG_INFO << "当前在线人数: " << g_onlineUsers.get();
    });


    loop.loop();       // 事件循环，无限循环处理事件

    return 0;
}
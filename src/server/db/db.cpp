#include "db.h"

#include<mysql/mysql.h>
#include<muduo/base/Logging.h>
#include<string>
#include<iostream>
using namespace std;


// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "Abc123456.";
static string dbname = "chatdata";


MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                          password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示？
        mysql_query(_conn, "set names utf8mb4");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }


    //重试一次
    if (p == nullptr) {
        LOG_INFO << "connect mysql fail! Retrying...";
        // 重试一次
        p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                          password.c_str(), dbname.c_str(), 3306, nullptr, 0);  // 复制参数
        if (p == nullptr) return false;
    }

    return true;

}

MYSQL_RES* MySQL::query(const std::string& sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << "query失败: " << mysql_error(_conn);
        return nullptr;
    }
    return mysql_use_result(_conn);
    
}

//更新操作
bool MySQL::update(const std::string& sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << "update失败: " << mysql_error(_conn);
        return false;
    }
    return true;
}




MYSQL* MySQL::getConnection(){
    return _conn;
}
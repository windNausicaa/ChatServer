#ifndef DB_H
#define DB_H


#include<mysql/mysql.h>
#include<string>
#include<vector>
using namespace std;


// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(const std::string& sql);
    // 查询操作
    MYSQL_RES* query(const std::string& sql);

    MYSQL* getConnection();

    // 安全转义字符串
    std::string escapeString(const std::string& str) {
        char escaped[ str.length()*2 + 1 ];
        mysql_real_escape_string(_conn, escaped, str.c_str(), str.length());
        return std::string(escaped);
    }
    
private:
    MYSQL *_conn;

    std::vector<long> _int_params;
};

#endif
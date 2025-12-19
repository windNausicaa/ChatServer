#include "usermodel.hpp"
#include "db.h"
#include <iostream>
#include<muduo/base/Logging.h>

using namespace std;

// User表的增加方法
/**
 * @brief 插入新用户到数据库
 * @param user 用户对象
 * @return 成功true，失败false
 */
bool UserModel::insert(User &user)
{
    // 1.组装sql语句
    // char sql[1024] = {0};
    // sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
    //         user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    char sql[1024] = {0};
    MySQL mysql;
    if (mysql.connect())
    {

        // 防止sql注入，安全措施
        std::string escaped_name = mysql.escapeString(user.getName());
        std::string escaped_pwd = mysql.escapeString(user.getPwd());

        sprintf(sql, "INSERT INTO User(name, password, state) VALUES('%s', '%s', '%s')",
                escaped_name.c_str(), escaped_pwd.c_str(), user.getState().c_str());

        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 根据用户号码查询用户信息
/**
 * @brief 根据用户号码查询用户信息
 * @param id 用户id
 * @return 成功返回用户对象，失败返回nullptr
 */
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", id);  // id数字安全

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                LOG_INFO << "row[0] "<< atoi(row[0]);
                user.setName(row[1] ? row[1] : "");
                LOG_INFO << "row[1] "<< row[1];
                user.setPwd(row[2] ? row[2] : ""); // 问题在这里
                LOG_INFO << "row[2] "<< row[2];
                LOG_INFO << "user.getPwd() "<< user.getPwd();

                user.setState(row[3] ? row[3] : "offline");
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}

// 更新用户的状态信息
/**
 * @brief 更新用户的状态信息
 * @param user 用户对象
 * @return 成功返回true，失败返回flase
 */
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d", 
            user.getState().c_str(), user.getId());  // state枚举安全

    MySQL mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}


// 重置用户的状态信息
/**
 * @brief 重置用户的状态信息
 */
void UserModel::resetState()
{
    // 1.组装sql语句
    // char sql[1024] = "update User set state = 'offline' where state = 'online'";

    std::string sql = "update User set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
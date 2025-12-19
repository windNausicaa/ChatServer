#include "offlinemessagemodel.hpp"
#include "db.h"
#include <cstring>

// 存储用户的离线消息
/**
 * @brief 存储用户的离线消息
 * @param userid 用户id 
 * @param msg 离线消息
 * @return 成功返回用户对象，失败返回nullptr
 */
void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    MySQL mysql;
    if (mysql.connect())
    {
        std::string escaped_msg = mysql.escapeString(msg);
        sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, escaped_msg.c_str());
        mysql.update(sql);
    }
}

// 删除用户的离线消息
/**
 * @brief 删除用户的离线消息
 * @param userid 用户id 
 * @return 
 */
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
/**
 * @brief 查询用户的离线消息
 * @param userid 用户id 
 * @return 用户消息数组
 */
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
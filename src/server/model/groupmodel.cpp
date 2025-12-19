#include "groupmodel.hpp"
#include "db.h"
#include <cstring>

// 创建群组
/**
 * @brief 创建群组
 * @param group 群组对象 
 * @return 成功返回true，失败返回false
 */
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    MySQL mysql;
    if (mysql.connect())
    {
        std::string escaped_name = mysql.escapeString(group.getName());
        std::string escaped_desc = mysql.escapeString(group.getDesc());
        sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
                escaped_name.c_str(), escaped_desc.c_str());

        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
/**
 * @brief 加入群组
 * @param userid 用户id 
 * @param groupid 群组id 
 * @param role 权限 
 * @return 
 */
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    MySQL mysql;
    if (mysql.connect())
    {
        std::string escaped_role = mysql.escapeString(role);
        sprintf(sql, "insert into GroupUser values(%d, %d, '%s')",
                groupid, userid, escaped_role.c_str());
        mysql.update(sql);
    }
}

// 查询用户所在群组信息
/**
 * @brief 查询用户所在群组信息
 * @param userid 用户id 
 * @param groupid 群组id 
 * @param role 权限 
 * @return 
 */
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join \
         GroupUser b on a.id = b.groupid where b.userid=%d", userid);

    vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    for (Group &group : groupVec)
    {
        char sql2[1024] = {0};
        sprintf(sql2, "select a.id,a.name,a.state,b.grouprole from User a \
            inner join GroupUser b on b.userid = a.id where b.groupid=%d", group.getId());

        MYSQL_RES *res = mysql.query(sql2);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
/**
 * @brief 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
 * @param userid 用户id 
 * @param groupid 群组id 
 * @return 返回群组用户id
 */
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);

    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}
//用户操作类，和用户类分开，这里专门实现对用户类的具体操作
//DAO

#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

//User表的数据操作类
class UserModel {
public:

    //数据库插入一条新用户数据
    bool insert(User &user);

    //查询用户信息
    User query(int id);

    // 更新用户的状态信息
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();
 
};


#endif
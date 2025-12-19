//用户操作类，和用户类分开，这里专门实现对用户类的具体操作
//DAO

#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
#include <openssl/sha.h> // 预防SQL注入，加密

//User表的数据操作类
class UserModel {
public:

    //数据库插入一条新用户数据
    bool insert(User &user);

    //查询用户信息
    User query(int id);

    //查询用户名是否重复

    // 更新用户的状态信息
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();

 
};


#endif
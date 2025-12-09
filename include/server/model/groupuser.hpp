#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

// 群组用户，直接继承User类，复用User的其他信息

class GroupUser : public User{

public:
   void setRole(string role) { this->role = role;}
   string getRole(){ return this->role; }

private:
   string role;
};

#endif
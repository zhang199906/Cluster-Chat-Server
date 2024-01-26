#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"

class GroupUser:public User
{
private:
    //相比user表中继承的信息，这里的信息多一个用户在组内的角色，可能是管理员或普通用户
    string role;
public:
    //设置角色
    void setRole(string role){this->role = role;}
    //查看角色
    string getRole(){return this->role;}
};
#endif
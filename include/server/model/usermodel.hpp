#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"

class UserModel{
public:
    //User表的增加方法,负责处理注册业务
    bool insert(User &user);
    //User表的查询方法,负责处理登录业务
    User query(int id);
    //更新用户的状态信息
    bool updateState(User user);
    //重置用户的状态信息
    void resetState();
};

#endif
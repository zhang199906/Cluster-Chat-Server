#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;

//User表的增加方法
bool UserModel::insert(User &user){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(), user.getPwd().c_str(),user.getState().c_str());

    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            //获取插入成功的用户数据生成的主键id,指定连接的上一条insert语句生成的主键
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }else{
        return false;
    }
    return false;
}

User UserModel::query(int id){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select * from User where id=%d",
        id);
    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            //获取查询到的行,可以用中括号运算符直接访问
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                return user;
            }
        }
    }
    User user;
    user.setId(-1);
    user.setName("");
    user.setPwd("");
    user.setState("");
    return user;
}

bool UserModel::updateState(User user){
    char sql[1024]={0};
    sprintf(sql,"update User set state='%s' where id = %d",user.getState().c_str(),user.getId());
    MySQL mysql;
    LOG_INFO << "更新用户状态";
    if(mysql.connect()){//必须先建立连接
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

void UserModel::resetState(){
    char sql[1024] = {0};
    sprintf(sql,"update User set state='offline' where state='online'");
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
#include "friendmodel.hpp"
#include "db.h"

//添加好友关系
void FriendModel::insert(int userid,int friendid){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into Friend values(%d,%d)",
        userid,friendid);

    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){
        mysql.update(sql);
    }else{
        LOG_INFO<<"连接数据库失败";
    }
}

//查询好友关系
vector<User> FriendModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on b.friendid=a.id where b.userid=%d",
        userid);
    vector<User> vec;
    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){//必须要有这一步获取chat库的连接
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            //把userid用户的所有friend放入vec中返回
            // LOG_INFO<<"friend 非空"<<mysql_fetch_row(res)[0];
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                // LOG_INFO<<atoi(row[0])<<"读取第一个";
                User user(atoi(row[0]),row[1],"",row[2]);
                // LOG_INFO<<atoi(row[0])<<row[1]<<row[2];
                vec.push_back(user);
            }
            LOG_INFO<<userid;
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
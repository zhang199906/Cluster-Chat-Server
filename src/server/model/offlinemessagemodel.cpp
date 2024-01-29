#include "offlinemessagemodel.hpp"
#include "db.h"
//#include <iostream>
//using namespace std;
//存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg){
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into OfflineMessage values('%d','%s')",
        userid, msg.c_str());

    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){//必须要有这一步获取chat库的连接
        mysql.update(sql);
    }
}
//删除用户的离线消息
void OfflineMsgModel::remove(int userid){
    char sql[1024] = {0};
    sprintf(sql,"delete from OfflineMessage where userid=%d",
        userid);

    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){//必须要有这一步获取chat库的连接
        mysql.update(sql);
    }
}
vector<string> OfflineMsgModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql,"select message from OfflineMessage where userid=%d",
        userid);
    vector<string> vec;
    MySQL mysql;
    LOG_INFO << sql;
    if(mysql.connect()){//必须要有这一步获取chat库的连接
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            //把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr){
//                cout<<row[0]<<endl;
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
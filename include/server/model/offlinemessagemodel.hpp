#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
//提供离线消息表的操作接口方法
#include <string>
#include <vector>
using namespace std;
class OfflineMsgModel{
public:
    //存储用户的离线消息
    void insert(int userid, string msg);
    //删除用户的离线消息
    void remove(int userid);
    //查询用户的离线消息
    vector<string> query(int userid);//返回一个存储离线消息的vector

};
#endif
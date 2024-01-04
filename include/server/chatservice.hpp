#ifndef CHATSERVICE_H
#define CHATSERVICE_H
//一个messageID对应一个回调函数
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include <iostream>
using namespace std;
using json = nlohmann::json;
using namespace muduo::net;
using namespace muduo;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp time)>;

//做业务有一个实例即可,故采用单例模式
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);
private:
    //构造函数私有化
    ChatService();
    //存储消息ID和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

};




#endif
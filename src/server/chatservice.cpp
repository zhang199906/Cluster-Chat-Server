#include "chatservice.hpp"
#include "public.hpp"
#include <iostream>
#include <string>

using namespace std;
#include <muduo/base/Logging.h>
using namespace muduo;
//获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

//存储消息id和对应的回调操作
ChatService::ChatService(){
    // _msgHandlerMap.insert();
    _msgHandlerMap.insert({LOGIN_MSG,bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({RES_MSG,bind(&ChatService::reg,this,_1,_2,_3)});
}

//处理登录业务   ORM框架 Object Relationship Map 业务层操作的都是对象，把mysql封装成对象，不想用mysql想用redis时，只要封装redis即可
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time){
    LOG_INFO<<"do login service!!!";
}

//处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp time){
    LOG_INFO<<"do reg service";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    LOG_INFO<<"准备注册";
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state){
        //注册成功
        LOG_INFO<<"注册成功";
        json response;
        response["msgid"] = RES_MSG_ACK;
        response["error"] = 0;
        response["id"] = user.getId();
        //给客户端返回注册成功的信息
        conn->send(response.dump());
    }else{
        //注册失败
        LOG_INFO<<"注册失败";
        json response;
        response["msgid"] = RES_MSG_ACK;
        response["error"] = 1;
        response["id"] = user.getId();
        //给客户端返回注册失败的信息
        conn->send(response.dump());
    }
}

//获取消息对应的处理器
MsgHandler ChatService::getMsgHandler(int msgId){
    auto it = _msgHandlerMap.find(msgId);
    //记录错误日志,msgid没有对应的事件处理回调
    if(it == _msgHandlerMap.end()){
        LOG_ERROR<<"msgId:"<<msgId<<"can not find handler!";
        //返回一个默认的处理器,这个处理器输出错误信息
        return [=](auto a,auto b,auto c){
            LOG_ERROR<<"msgId:"<<msgId<<"can not find handler!";
        };
    }
    else{
        return _msgHandlerMap[msgId];
    }
}
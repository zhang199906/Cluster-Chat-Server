#ifndef PUBLIC_H
#define PUBLIC_H
/*
server和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1, //登录消息
    LOGIN_MSG_ACK, //登录响应消息
    RES_MSG, //注册消息,自增
    RES_MSG_ACK, //注册响应消息
    ONE_CHAT_MSG //聊天消息 
};



#endif
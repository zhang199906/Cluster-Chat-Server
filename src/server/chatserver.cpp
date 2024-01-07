#include "chatserver.hpp"
#include <functional>
#include "json.hpp"
#include <string>
#include <iostream>
#include "chatservice.hpp"
using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
            const InetAddress& listenAddr,
            const string& nameArg):_server(loop,listenAddr,nameArg)
{
    //注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
    //设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start(){
    _server.start();
}
//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn){
    if(!conn->connected()){
        conn->shutdown();  //客户端断开链接
    }
}
//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,Buffer *buffer,Timestamp time){
    string buf = buffer->retrieveAllAsString(); //把Buffer转化为string;
    //数据的反序列化
    json js = json::parse(buf);
    //目的:完全解耦网络模块的代码和业务模块的代码
    //方法:1.面向接口的编程 2.面向抽象基类/回调操作
    //通过js中的messageID,调用绑定的回调操作,i.e.获取一个处理器handler=>>conn js time
    MsgHandler msgHandler =ChatService::instance()->getMsgHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器,来执行响应的业务处理
    msgHandler(conn,js,time);
}
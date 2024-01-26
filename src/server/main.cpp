#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;
//处理服务器ctrl+c结束后，充值user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}


int main(){
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();  //服务器启动
    loop.loop();   //开启事件循环
    return 0;
}
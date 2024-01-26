#ifndef CHATSERVICE_H
#define CHATSERVICE_H
//一个messageID对应一个回调函数
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include <iostream>
// #include "public.hpp"
using namespace std;
using json = nlohmann::json;
using namespace muduo::net;
using namespace muduo;
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
using MsgHandler = std::function<void(const TcpConnectionPtr &,json &,Timestamp)>;

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
    //一对一聊天功能 
    void oneChat(const TcpConnectionPtr $conn, json &js, Timestamp time);
    //获取消息ID对应的处理器
    MsgHandler getMsgHandler(int msgId);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常业务重置方法，如异常退出时重置用户为offline
    void reset();
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群组业务
    void joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群聊功能
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
private:
    //构造函数私有化
    ChatService();
    //存储消息ID和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;
    //存储在线用户的连接
    unordered_map <int,TcpConnectionPtr> _userConnMap;
    //定义互斥锁,保证线程安全
    mutex _connMutex;
    //数据操作类对象,用户登录，注册信息表
    UserModel _userModel;
    //离线消息表
    OfflineMsgModel _offlineMsgModel;
    //好友信息表
    FriendModel _friendModel;
    //群组相关表操作
    GroupModel _groupModel;
};




#endif
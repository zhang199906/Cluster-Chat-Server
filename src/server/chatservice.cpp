#include "chatservice.hpp"
#include "public.hpp"
#include <iostream>
#include <string>

using namespace std;
#include <muduo/base/Logging.h>
#include "friendmodel.hpp"
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
    _msgHandlerMap.insert({ONE_CHAT_MSG,bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,bind(&ChatService::addFriend,this,_1,_2,_3)});
}

//处理登录业务   ORM框架 Object Relationship Map 业务层操作的都是对象，把mysql封装成对象，不想用mysql想用redis时，只要封装redis即可
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time){
    LOG_INFO<<"do login service!!!";
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getPwd() == pwd){
        if(user.getState() == "online"){
            //该用户已经登录,不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 2;
            response["errormsg"] = "该账号已经登录,请重新输入新账号";
            conn->send(response.dump());
        }else{
            //登录成功,记录用户连接信息
            //使用带智能指针的锁自动解锁
            {lock_guard<mutex> lock(_connMutex);
            _userConnMap.insert({id,conn});}
            //登录成功,更新用户状态信息
            user.setState("online");
            if(_userModel.updateState(user)){
                LOG_INFO<<"修改登录状态成功";
            }else{
                LOG_INFO<<"修改登录状态失败";
            }
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"]=vec;
                //读取该用户的离线消息以后，把该用户所有的离线消息删除掉
                _offlineMsgModel.remove(id);
            }
            //查询该用户的好友信息并返回
            vector<User> friendVec = _friendModel.query(id);
            if(!friendVec.empty()){
                vector<string> friendToString;
                for(User &user : friendVec){
                    json js;
                    js["id"]=user.getId();
                    js["name"]=user.getName();
                    js["state"]=user.getState();
                    friendToString.push_back(js.dump());
                }
                response["friends"] = friendToString;
            }
            conn->send(response.dump());
        }
    }else{
        //登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["error"] = 1;
        response["errormsg"] = "登录失败";
        conn->send(response.dump());
    }

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

void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {lock_guard<mutex> lock(_connMutex);
    for(auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
    {
        if(it->second == conn){
            //从map表删除用户的连接信息
            user.setId(it->first);
            _userConnMap.erase(it);
            break;
        }
    }
    }
    if(user.getId() != -1){
        //更新用户的状态信息
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr $conn, json &js, Timestamp time){
    // int toid = js["to"].get<int>();
    int toid = js["to"];
    unordered_map<int, TcpConnectionPtr>::iterator it;
    //标识用户是否在线
    bool userState = false;
    {
        lock_guard<mutex> lock(_connMutex);
        it = _userConnMap.find(toid);
        if(it != _userConnMap.end()){
            //toid在线,转发消息
            userState = true;
        }
    }
    if(userState){
        //toid在线,转发消息
        it->second->send(js.dump());//服务器相当于做了一次消息转发
    }else{
        //toid不在线,存储离线消息
        _offlineMsgModel.insert(toid,js.dump());
    }
}

void ChatService::reset(){
    //把online状态的用户，设置成offline
    _userModel.resetState();
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"];
    //或
    // int userid = js["id"].get<int>();
    int friendid = js["friendid"];
    //存储好友信息
    _friendModel.insert(userid,friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"];
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];
    Group group;
    group.setName(groupname);
    group.setDesc(groupdesc);
    if(_groupModel.createGroup(group)){
        js["groupid"] = group.getId();
        js["grouprole"] = "creator";
        joinGroup(conn,js,time);
    }
}

void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = js["id"];
    string grouprole = js["grouprole"];
    _groupModel.joinGroup(userid,groupid,grouprole);
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"];
    int groupid = js["id"];
    vector<int> vecUser = _groupModel.queryGroupUsers(userid,groupid);
    for(int id : vecUser){
        if(_userConnMap.find(id) != _userConnMap.end()){
            _userConnMap[id]->send(js.dump());
        }else{
            _offlineMsgModel.insert(id,js.dump());
        }
    }
}
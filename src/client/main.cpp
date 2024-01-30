#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

//记录当前系统登录的用户信息
User currentUser;
//记录当前登录用户的好友列表信息
vector<User> currentUserFriendList;
//记录当前登录的用户的群组列表信息
vector<Group> currentUserGroupList;
//显示当前登录成功的用户的基本信息
void showCurrentUserData();

//控制主菜单页面程序的bool值
bool isMainMenuRunning = false;
//接收线程
void readTaskHandler(int clientfd);
//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
//主聊天页面程序
void mainMenu(int clientfd);


//聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc,char **argv)
{
    if(argc < 3){
        cerr << "command invalid! example: .ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }
    
    //解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建client的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1){
        cerr << "socket create error" << endl;
        exit(-1);
    }

    //填写client需要连接的server信息ip+port
    sockaddr_in server;
    //将server中的值初始化为0
    memset(&server,0,sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    //client和server进行连接
    if(connect(clientfd, (sockaddr*)&server, sizeof(sockaddr_in)) == -1)
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    //main线程用于接收用户输入，负责发送数据
    for(;;)
    {
        //显示首页菜单、登录、注册、退出
        cout << "====================" << endl;
        cout << "1.login" << endl;
        cout << "2.rigister" << endl;
        cout << "3.quit" << endl;
        cout << "====================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); //读掉缓冲区残留的回车

        switch (choice)
        {
            //login业务
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); //读掉缓冲区的回车
            cout << "userpassword:";
            cin.getline(pwd,50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd,request.c_str(), strlen(request.c_str())+1, 0);
            if(len == -1)
            {
                cerr << "send login message error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if(len == -1)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
//                    cout << buffer << endl;
                    json responsejs = json::parse(buffer);
                    if(responsejs["error"] != 0){ //登录失败
                        cerr << responsejs["errmsg"] <<endl;
                    }
                    else //登录成功
                    {
                        currentUserGroupList.clear();
                        currentUserFriendList.clear();
                        //记录当前用户的id和name
//                        cout<<buffer;

                        currentUser.setId(responsejs["id"]);
                        currentUser.setName(responsejs["name"]);

                        //记录当前用户的好友列表
                        if(responsejs.contains("friends"))
                        {
                            vector<string> vec = responsejs["friends"];
                            for(string str : vec)
                            {
                                json friendjs = json::parse(str);
                                User user;
                                user.setId(friendjs["id"]);
                                user.setName(friendjs["name"]);
                                user.setState(friendjs["state"]);
                                currentUserFriendList.push_back(user);
                            }
                        }

                        //记录当前用户的群组列表信息
                        if(responsejs.contains("groups"))
                        {
                            //初始化
                            vector<string> vecGroup = responsejs["groups"];
                            for(string groupstr : vecGroup)
                            {
                                json groupjs = json::parse(groupstr);
                                Group group;
                                group.setId(groupjs["id"]);
                                group.setName(groupjs["groupname"]);
                                group.setDesc(groupjs["groupdesc"]);
                                
                                vector<string> vecGroupUsers = groupjs["users"];
                                for(string userstr : vecGroupUsers)
                                {
                                    GroupUser user;
                                    json userjs = json::parse(userstr);
                                    user.setId(userjs["id"]);
                                    user.setName(userjs["name"]);
                                    user.setState(userjs["state"]);
                                    user.setRole(userjs["role"]);
                                    group.getUsers().push_back(user);
                                }

                                currentUserGroupList.push_back(group);
                            }
                        }

                        //显示登录用户的基本信息
                        showCurrentUserData();

                        //显示当前用户的离线消息  个人聊天信息或者群主消息
                        if(responsejs.contains("offlinemsg"))
                        {
                            vector<string> offlineMsgVec = responsejs["offlinemsg"];
                            for(string offlineMsg : offlineMsgVec)
                            {
                                json msgjs = json::parse(offlineMsg);
                                //time + [id] + name + said + xxx
                                int msgid = msgjs["msgid"];
                                if(msgid == ONE_CHAT_MSG)
                                {
                                    cout << msgjs["time"] << "[" << msgjs["id"]<< "]"  << msgjs["name"] << "said：" << msgjs["msg"] <<endl;
                                }
                                else if(msgid == GROUP_CHAT_MSG)
                                {
                                    cout << "群消息 [" << msgjs["groupid"] << "] " << msgjs["time"] << " [" << msgjs["id"] << "] " << msgjs["name"]
                                         << "said: " << msgjs["msg"] << endl;
                                }
                            }
                        }

                        //登录成功，启动接收线程负责接收数据
                        static int readthreadnumber = 0;
                        if(readthreadnumber == 0){
                            thread readTask(readTaskHandler, clientfd);   //pthread_create
                            readTask.detach();     //pthread_detach
                            readthreadnumber++;
                        }

                        //进入聊天主菜单页面
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2:
        {
            //处理注册业务
            char pwd[50] = {0};
            char name[50] = {0};
            cout << "开始注册账号，请按照提示信息输入：" << endl;
            cout << "name:";
            cin.getline(name,50);
//            cin.get(); //读掉缓冲区的回车
            cout << "userpassword:";
            cin.getline(pwd,50);

            json js;
            js["msgid"] = RES_MSG;
            js["name"] = name; 
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd,request.c_str(), strlen(request.c_str())+1, 0);
            if(len == -1)
            {
                cerr << "send login message error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if(len == -1)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if(responsejs["error"] != 0){ //登录失败
                        cerr << "注册失败"<<endl;
                    }
                    else //注册成功
                    {
                        int userid = responsejs["id"];
                        cout << "your userid is:" << userid << ", Please remember it." << endl;
                    }
                }
            }

        }
        break;
        case 3:
        {
            exit(0);
        }
        break;
        default:
        {
            cerr << "input invalid" << endl;
        }
        break;
        }
    }
}

//显示当前登录成功的用户的基本信息
void showCurrentUserData()
{
    cout << "====================login user====================" << endl;
    cout << "current login user => id:"<<currentUser.getId() << "name:" <<currentUser.getName() << endl;
    cout << "--------------------friend list--------------------" << endl;
    if(!currentUserFriendList.empty())
    {
        for(User user : currentUserFriendList)
        {
            cout << user.getId() << " " <<user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "--------------------group list---------------------" << endl;
    if(!currentUserGroupList.empty())
    {
        for(Group group : currentUserGroupList)
        {
            cout << "群组名称" << group.getId() << " " << group.getName() << " " <<group.getDesc() <<endl;
            for(GroupUser user : group.getUsers())
            {
                cout << user.getId() << " " <<user.getName() << " " << user.getState() << " "
                << user.getRole() <<endl;
            }
        }
    } 
    cout << "===================================================" << endl;

}

void readTaskHandler(int clientfd){
    for(;;)
    {
        char buffer[1024] = {0};
        //阻塞地读取套接字中发过来的消息
        int len = recv(clientfd,buffer,1024,0);
        if(len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }

        //接收ChatServer转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        int msgid = js["msgid"];
        if(msgid == ONE_CHAT_MSG)
        {
//            cout<<buffer<<endl;
            cout << js["time"] << "[" << js["id"]<< "]"  << js["name"] << "said：" << js["msg"] <<endl;
            continue;
         }
        else if(msgid == GROUP_CHAT_MSG)
        {
            cout << "群消息 [" << js["groupid"] << "] " << js["time"] << " [" << js["id"] << "] " << js["name"]
            << "said: " << js["msg"] << endl;
            continue;
        }
    }
}

//command handler
void help(int fd = 0,string str = "");

void chat(int,string);

void addfriend(int,string);

void creategroup(int,string);

void joingroup(int,string);

void groupchat(int,string);

void logout(int,string);

//系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
        {"help","显示所有支持的命令，格式help"},
        {"chat","一对一聊天，格式chat:frienidd:message"},
        {"addfriend","添加好友，格式addfriend:friendid"},
        {"creategroup","创建群组，格式creategroup:groupname:groupdesc"},
        {"joingroup","加入群组，格式joingroup:groupid"},
        {"groupchat","群聊，格式groupchat:groupid:message"},
        {"logout","注销，格式logout"},
};

//注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
        {"help",help},
        {"chat",chat},
        {"addfriend", addfriend},
        {"creategroup", creategroup},
        {"joingroup", joingroup},
        {"groupchat", groupchat},
        {"logout",logout}

};

void mainMenu(int clientfd){
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning){
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command; //存储命令
        int idx = commandbuf.find(":");
        if(idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx);
        }
//        cout << command <<endl;
        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr << "invalid input comman!" << endl;
            continue;
        }

//        cout << commandbuf.substr(idx+1, commandbuf.size() - idx) << endl ;
        //调用相应的命令处理回调事件，mainMenu对修改封闭，添加新功能不需要修改该函数cha
        it->second(clientfd,commandbuf.substr(idx + 1,commandbuf.size() - idx)); //调用命令处理方法

    }
}


//"help" command handler
void help(int,string){
    cout << "show command list >>>" << endl;
    for(auto p : commandMap)
    {
        cout << p.first << "." << p.second << endl;
    }
    cout << endl;
}

//"addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "send addfriend msg error" << js.dump().c_str() << endl;
    }
}

//"chat" command handler
void chat(int clientfd, string str)
{
//    int friendid = atoi(str.c_str());
    json js;
    int idx = str.find(":");
    if(idx == -1){
        cerr << "error: friendid not found" << endl;
        return;
    }
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
//    js["friendid"] = friendid;
//    string buffer = js.dump();

    js["to"] = atoi(str.substr(0,idx).c_str());
    js["msg"] = str.substr(idx+1, str.size() - idx);
    js["time"] = getCurrentTime();
    string buffer = js.dump().c_str();
//    cout<<buffer<<endl;

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "send chat msg error" << js.dump().c_str() << endl;
    }
}

void creategroup(int clientfd,string str){
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr << "create group command invalid!" << endl;
        return;
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr << "send create group msg error -> " << buffer << endl;
    }
}

void joingroup(int clientfd,string str){
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = JOIN_GROUP_MSG;
    js["groupid"] = groupid;
    js["id"] = currentUser.getId();
    js["grouprole"] = "normal";
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(), strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr << "send join group msg error -> " << buffer << endl;
    }
}

void groupchat(int clientfd,string str){
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr << "group chat command invalid!" << endl;
        return;
    }
    int groupid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx + 1, strlen(str.c_str())-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "send group chat msg error -> "<< buffer << endl;
    }
}

//"logout" command handler
void logout(int clientfd,string){
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1,0);
    if(len == 1)
    {
        cerr << "send logout msg error" << endl;
    }
    isMainMenuRunning = false;
}

string getCurrentTime(){
    time_t now = time(0);
    string date(ctime(&now));

    return date.substr(0,date.size()-1);
}
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


//接收线程
void readTaskHandler(int clientfd);
//获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();
//主聊天页面程序
void mainMenu();

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
            cout << "userid";
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
                    json responsejs = json::parse(buffer);
                    if(responsejs["error"] != 0){ //登录失败
                        cerr << responsejs["errmsg"] <<endl;
                    }
                    else //登录成功
                    {
                        //记录当前用户的id和name
                        currentUser.setId(responsejs["id"]);
                        currentUser.setName(responsejs["name"]);

                        //记录当前用户的好友列表
                        if(responsejs.contains("friends"))
                        {
                            vector<string> vec = responsejs["friends"];
                            for(string str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"]);
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                currentUserFriendList.push_back(user);
                            }
                        }

                        //记录当前用户的群组列表信息
                        if(responsejs.contains("groups"))
                        {
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
                                cout << msgjs["time"] << "[" << msgjs["id"] << "]" << msgjs["name"]
                                << "said:" << js["msg"] << endl;
                            }
                        }

                        //登录成功，启动接收线程负责接收数据
                        thread readTask(readTaskHandler, clientfd);
                        readTask.detach();
                        //进入聊天
                        mainMenu();
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
            cin >> name;
            cin.get(); //读掉缓冲区的回车
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
            cerr << "input invalid" << endl;
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
            cout << user.getId() << " " <<user.getName() << " " << user.getState();
        }
    }
    cout << "--------------------group list---------------------" << endl;
    if(!currentUserGroupList.empty())
    {
        for(Group group : currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " <<group.getDesc() <<endl;
            for(GroupUser user : group.getUsers())
            {
                cout << user.getId() << " " <<user.getName() << " " << user.getState() 
                << user.getRole() <<endl;
            }
        }
    } 
    cout << "===================================================" << endl;

}

void readTaskHandler(int clientfd){

}

void mainMenu(){
    
}
#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include <vector>
#include <user.hpp>
#include <group.hpp>

class GroupModel
{
public:
    //新建一个群组
    // bool createGroup(Group &group);
    bool createGroup(Group &group);
    //查询用户所在群组的信息，除了该用户的ID信息，用于群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
    //查询用户所在的所有群组
    vector<Group> queryGroups(int userid);
    //加入群组
    void joinGroup(int userid,int groupid,string role);
};
#endif
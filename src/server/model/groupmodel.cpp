#include "groupmodel.hpp"
#include "db.h"

bool GroupModel::createGroup(Group &group){
    char sql[1024];
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::joinGroup(int userid, int groupid, string role){
    char sql[1024];
    sprintf(sql,"insert into GroupUser values(%d,%d,'%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//vector<Group> GroupModel::queryGroups(int userid){
//    char sql[1024] = {0};
//    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid= %d;",userid);
//    MySQL mysql;
//    vector<Group> vec;
//    if(mysql.connect()){
//        MYSQL_RES* res = mysql.query(sql);
//        if(res != nullptr) {
//            MYSQL_ROW row;
//            while ((row = mysql_fetch_row(res)) != nullptr) {
//                Group group;
//                group.setId(atoi(row[0]));
//                group.setName(row[1]);
//                group.setDesc(row[2]);
////                sprintf(sql,
////                        "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid=a.id where b.groupid=%d",
////                        group.getId());
//                sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.groupid=%d;",group.getId());
//                MYSQL_RES *resUser = mysql.query(sql);
//                MYSQL_ROW rowUser;
//                while ((rowUser = mysql_fetch_row(resUser)) != nullptr) {
//                    GroupUser user;
//                    user.setId(atoi(rowUser[0]));
//                    user.setName(rowUser[1]);
//                    user.setState(rowUser[2]);
//                    user.setRole(rowUser[3]);
//                    group.getUsers().push_back(user);
//                }
//                mysql_free_result(resUser);
//                vec.push_back(group);
//            }
//        }
//        mysql_free_result(res);
//    }
//    return vec;
//}
//把上一个mysql查询到的内容全部读取完再select
vector<Group> GroupModel::queryGroups(int userid){
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid= %d",userid);
    MySQL mysql;
    vector<Group> vec;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
        }
    }
    for(Group &group : vec)
    {
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.groupid=%d",group.getId());
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}

vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    char sql[1024];
    sprintf(sql,"select userid from GroupUser where groupid=%d",groupid);
    MySQL mysql;
    vector<int> vec;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        LOG_INFO << sql;
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr) {
                if ((atoi(row[0]) != userid)) {
                    vec.push_back(atoi(row[0]));
                }
            }
        }
        mysql_free_result(res);
        return vec;
    }
    return vec;
}
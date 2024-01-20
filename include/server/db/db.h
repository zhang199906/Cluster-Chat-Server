//
// Created by root on 1/17/24.
//

#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>
using namespace std;
//数据库配置信息
static string server_ = "sh-cynosdbmysql-grp-qjyz35w2.sql.tencentcdb.com";
static string user_ = "root";
static string dbname_ = "chat";
static string password_ = "Zh990606";
static int port_ = 21420;

//数据库操作类
class MySQL{
public:
    //初始化数据库连接
    MySQL();
    //释放数据库连接资源
    ~MySQL();
    //连接数据库
    bool connect();
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
    //获取连接
    MYSQL* getConnection();
private:
    MYSQL *_conn;
};

#endif //DB_H

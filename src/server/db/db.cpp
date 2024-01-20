//
// Created by root on 1/18/24.
//
#include "db.h"
MySQL::MySQL() {
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL(){
    if(_conn != nullptr){
        mysql_close(_conn);
    }
}

bool MySQL::connect(){
    MYSQL *p = mysql_real_connect(_conn, server_.c_str(), user_.c_str(), password_.c_str(), dbname_.c_str(), port_, nullptr,0);
    if (p != nullptr){
        //C和C++代码默认的编码字符是ASCII,如果不设置,从MySQL上拉下来的中文显示?
        mysql_query(_conn,"set names gbk");
        LOG_INFO << "connect mysql success";
    }else{
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}

bool MySQL::update(string sql){
    if (mysql_query(_conn, sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__<< ":"
                    << sql << "更新失败";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(string sql){
    if (mysql_query(_conn,sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__<< ":"
                 << sql << "查询失败";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection(){
    return _conn;
}
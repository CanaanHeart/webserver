#ifndef _MYSQLCONN_H_
#define _MYSQLCONN_H_

#include "../logger/Logger.h"
#include "../time/TimeStamp.h"

#include <mysql/mysql.h>

#include <iostream>
#include <string> 

class MysqlConn
{
private:
    MYSQL *sql_ = nullptr;
    MYSQL_RES *res_ = nullptr;
    MYSQL_ROW row_ = nullptr;
    TimeStamp tim_;
public:
    MysqlConn();
    ~MysqlConn();
    bool Connect(std::string user, std::string passwd, std::string db_name, std::string ip, uint16_t port = 3306);
    bool Update(std::string sql_statement);
    bool Query(std::string sql_statement);
    bool Next();
    std::string Value(int index);
    bool TransAction();
    bool Commit();
    bool RoolBack();
    void RefreshAliveTime();
    time_t GetAliveTime();
private:
    void FreeResult();
};

#endif // _MYSQLCONN_H_
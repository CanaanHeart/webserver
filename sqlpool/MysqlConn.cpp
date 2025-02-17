#include "MysqlConn.h"

using namespace std;

MysqlConn::MysqlConn()
{
    sql_ = mysql_init(nullptr);
    mysql_set_character_set(sql_, "utf8");
}

MysqlConn::~MysqlConn()
{
    if(sql_)
        mysql_close(sql_);
    FreeResult();
}

bool MysqlConn::Connect(std::string user, std::string passwd, std::string db_name, std::string ip, uint16_t port)
{
    MYSQL* ptr = mysql_real_connect(sql_, ip.c_str(), user.c_str(), passwd.c_str(), db_name.c_str(), port, nullptr, 0);
    if(ptr)
        return true;
    else
        return false;
}

bool MysqlConn::Update(std::string sql_statement)
{
    int ret = mysql_query(sql_, sql_statement.c_str());
    if(ret != 0)
        return false;
    return true;
}

bool MysqlConn::Query(std::string sql_statement)
{
    FreeResult();
    int ret = mysql_query(sql_, sql_statement.c_str());
    if(ret != 0)
        return false;

    res_ = mysql_store_result(sql_);
    if(!res_)
        return false;
    
    int rows = mysql_num_rows(res_);
    if(rows == 0)
        return false;
    return true;
}

bool MysqlConn::Next()
{
    if(!res_)
        return false;
    int rows = mysql_num_rows(res_);
    if(rows == 0)
        return false;
    row_ =  mysql_fetch_row(res_);
    if(row_)
        return true;
    return false;
}

string MysqlConn::Value(int index)
{
    int field_nums = mysql_num_fields(res_);
    if(index < 0 || index >= field_nums){
        WARN("index valid!");
        return string();
    }
    char *val = row_[index];
    uint64_t len = mysql_fetch_lengths(res_)[index];
    return string(val, len);
}

bool MysqlConn::TransAction()
{
    int ret = mysql_autocommit(sql_, false);
    if(ret != 0)
        return false;
    return true;
}

bool MysqlConn::Commit()
{
    int ret = mysql_commit(sql_);
    if(ret != 0)
        return false;
    return true;
}

bool MysqlConn::RoolBack()
{
    int ret = mysql_rollback(sql_);
    if(ret != 0)
        return false;
    return true;
}

void MysqlConn::FreeResult()
{
    if(res_){
        mysql_free_result(res_);
        res_ = nullptr;
    }
}

void MysqlConn::RefreshAliveTime()
{
    tim_ = TimeStamp::Now();
}

time_t MysqlConn::GetAliveTime()
{
    return time(0) - tim_.GetTimeInt();
}
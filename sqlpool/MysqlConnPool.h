#ifndef _MYSQLCONNPOOL_H_
#define _MYSQLCONNPOOL_H_

#include "../logger/Logger.h"
#include "MysqlConn.h"
#include "SqlLoadConfig.h"

#include <iostream>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <map>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>

using SP_MysqlConn = std::shared_ptr<MysqlConn>;

class MysqlConnPool
{
private:
    SqlLoadConfig slc_;
    std::string ip_;
    std::string user_;
    std::string passwd_;
    std::string db_name_;
    uint16_t port_;
    int max_conns_;         // 最大连接数
    int min_conns_;         // 最小连接数
    int curr_conns_;        // 当前连接数
    int timeout_;           // 超时时间
    int idle_time_;         // 最大空闲时间
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<MysqlConn*> conns_;
    bool flag_ = true;
private:
    MysqlConnPool();
    void Init();
    // std::map<std::string, std::string> InitConfig();
    void ProduceMysqlConn();
    void DestructMysqlConn();
    void CreateConn();
public:
    ~MysqlConnPool();
    MysqlConnPool(const MysqlConnPool&) = delete;
    MysqlConnPool& operator=(const MysqlConnPool&) = delete;
    static MysqlConnPool& GetInstance();
    SP_MysqlConn GetConn();

    void Print();
};

#endif // _MYSQLCONNPOOL_H_
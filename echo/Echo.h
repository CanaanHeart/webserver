#ifndef _ECHO_H_
#define _ECHO_H_

#include "../logger/Logger.h"
#include "../inc/TcpServer.h"
#include "../inc/EventLoop.h"
#include "../inc/Connection.h"
#include "../threadpool/ThreadPool.h"
#include "Buffer.h"

#include <iostream>
#include <string>

class EchoServer{
private:
    TcpServer server;
    ThreadPool m_tp;
public:
    EchoServer(const uint16_t port, const std::string *ip = nullptr, 
        const int tp_nums = std::thread::hardware_concurrency() / 2, 
        const int work_ths = std::thread::hardware_concurrency() / 2);
    ~EchoServer() = default;

    void Start();
    void Stop();

    void NewConnection(SP_Connection conn);
    void DeleteConnection(SP_Connection conn);
    void ErrorConnection(SP_Connection conn);
    void OnMessage(SP_Connection conn, Buffer &buf); // 将业务添加到工作线程池
    void SendComplete(SP_Connection conn);

    void Task(SP_Connection conn, Buffer &buf);  // 业务处理函数
};

#endif // _ECHO_H_
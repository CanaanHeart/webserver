#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "../logger/Logger.h"
#include "../inc/TcpServer.h"
#include "../inc/EventLoop.h"
#include "../inc/Connection.h"
#include "../inc/Buffer.h"
#include "../threadpool/ThreadPool.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpLoadConfig.h"

#include <iostream>
#include <string>
#include <mutex>
#include <memory>

class HttpServer
{
private:
    std::unique_ptr<TcpServer> server_;
    std::unique_ptr<ThreadPool> work_tp_;
    std::mutex mtx_;
    std::mutex user_mtx_;
    std::string server_dir_;
public:
    HttpServer(const uint16_t port, const std::string &ip, const std::string &server_dir);
    ~HttpServer() = default;

    void Start();
    void Stop();

    void NewConnection(SP_Connection conn);
    void DeleteConnection(SP_Connection conn);
    void ErrorConnection(SP_Connection conn);
    void OnMessage(SP_Connection conn, Buffer &buf); // 将业务添加到工作线程池
    void SendComplete(SP_Connection conn);

    void Task(SP_Connection conn, Buffer buf);  // 业务处理函数
};

#endif // _HTTPSERVER_H_
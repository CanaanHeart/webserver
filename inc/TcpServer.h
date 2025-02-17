#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include "../logger/Logger.h"
#include "../threadpool/ThreadPool.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"
#include "Buffer.h"

#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

using SP_Connection = std::shared_ptr<Connection>;
using UP_EventLoop = std::unique_ptr<EventLoop>;

class TcpServer{
private:
    UP_EventLoop main_loop_;
    std::vector<UP_EventLoop> sub_loops_;
    Acceptor accp_;
    std::unique_ptr<ThreadPool> tp_;
    std::map<int, SP_Connection> conns_;
    std::mutex mtx_;

    std::function<void(SP_Connection)> new_connection_cb_;
    std::function<void(SP_Connection)> delete_connection_cb_;
    std::function<void(SP_Connection)> error_connection_cb_;
    std::function<void(SP_Connection, Buffer&)> on_message_cb_;
    std::function<void(SP_Connection)> send_complete_cb_;
public:
    TcpServer(const uint16_t port, const std::string &ip);
    ~TcpServer() = default;

    void Start();
    void Stop();
    void NewConnection(UP_Socket cli_sock);   // 处理新客户端的连接请求，在Acceptor中回调
    void DeleteConnection(SP_Connection conn);    // 关闭客户端连接，在Connection中回调
    void DeleteConnectionEp(SP_Connection conn);  // 用于在EventLoop中Connection超时进行回调
    void ErrorConnection(SP_Connection conn); // 客户端连接错误，在Connection中回调
    void OnMessage(SP_Connection conn, Buffer &buf);  // 处理客户端的请求包文，在Connection中回调
    void SendComplete(SP_Connection conn);    // 数据发送完成，通知TcpServer，在Connection中回调

    void SetNewConnetionCB(std::function<void(SP_Connection)> fn);
    void SetDeleteConnectionCB(std::function<void(SP_Connection)> fn);
    void SetErrorConnectionCB(std::function<void(SP_Connection)> fn);
    void SetOnMessageCB(std::function<void(SP_Connection, Buffer&)> fn);
    void SetSendCompleteCB(std::function<void(SP_Connection)> fn);

};

#endif // _TCPSERVER_H_
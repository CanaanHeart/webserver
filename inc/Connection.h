#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "../logger/Logger.h"
#include "../time/TimeStamp.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Buffer.h"

#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/syscall.h>

#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <atomic>

class EventLoop;
class Channel;
class Connection;

#define INT_BYTES 4

using SP_Connection = std::shared_ptr<Connection>;
using UP_Socket = std::unique_ptr<Socket>;
using UP_Channel = std::unique_ptr<Channel>;
using UP_Buffer = std::unique_ptr<Buffer>;

class Connection: public std::enable_shared_from_this<Connection>
{
private:
    EventLoop *loop_;
    UP_Socket cli_sock_;
    UP_Channel cli_ch_;
    std::atomic_bool is_disconnetced_; // true，断开
    UP_Buffer read_buf_;
    UP_Buffer send_buf_;
    TimeStamp tim_;    // 时间戳，每接收到一个报文就更新时间
    
    std::function<void(SP_Connection)> close_cb_;    // 关闭m_fd的回调函数，回调TcpServer::DeleteConnection
    std::function<void(SP_Connection)> error_cb_;    // m_fd上发生错误的回调函数，回调TcpServer::ErrorConnection
    std::function<void(SP_Connection, Buffer&)> on_message_cb_; // 处理报文回调函数，回调TcpServer::OnMessage
    std::function<void(SP_Connection)> send_complete_cb_;    // 发送数据完成后的回调函数，回调TcpServer::SendComplete
public:
    Connection(EventLoop *loop, UP_Socket cli_sock);
    ~Connection(){}

    int GetFd() const; 
    uint16_t GetPort() const;
    std::string GetIp() const;
    void SetSendBuffer(Buffer &buf);
    void SendInLoop(Buffer &buf);  // 在I/O线程中发送数据

    void SetCloseCB(std::function<void(SP_Connection)> fn);
    void SetErrorCB(std::function<void(SP_Connection)> fn);
    void SetOnMessageCB(std::function<void(SP_Connection, Buffer&)> fn);
    void SetSendCompleteCB(std::function<void(SP_Connection)> fn);
    bool IsTimeout(time_t now, time_t timeout = 10);

    void OnMessage();   // 读取数据，供Channel回调
    void CloseCB(); // TCP连接关闭回调，供Channel回调
    void ErrorCB(); // TCP连接错误回调，供Channel回调
    void WriteCB(); // 向客户端写数据，供Channel回调
};

#endif // _CONNECTION_H_
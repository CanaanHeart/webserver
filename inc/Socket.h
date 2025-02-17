#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "../logger/Logger.h"
#include "InetAddress.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>    // TCP_NODELAY解决网络拥塞

#include <iostream>
#include <string>

class Socket
{
private:
    int fd_;
    uint16_t port_;
    std::string ip_;
public:
    Socket();
    Socket(int fd);
    ~Socket();
    void Bind(const InetAddress &addr);
    void Listen(int n = SOMAXCONN);
    int SetNonBlock();
    bool IsNonBlock();
    int Accept(InetAddress &addr);
    void Connect(const InetAddress &addr);
    int GetFd() const { return fd_; }
    uint16_t GetPort() const { return port_; }
    std::string GetIp() const { return ip_; }
    void SetPortIp(const uint16_t &port, const std::string &ip);

    void SetReuseAddr(bool on = true);
    void SetReusePort(bool on = true);
    void SetNoDelay(bool on = true);
    void SetKeepAlive(bool on = true);
};

#endif // _SOCKET_H_
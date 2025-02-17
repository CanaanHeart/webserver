#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "../logger/Logger.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "Connection.h"

#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <iostream>
#include <string>
#include <functional>
#include <memory>

class EventLoop;

class Acceptor{
private:
    EventLoop *loop_;
    Socket serv_sock_;
    Channel serv_ch_;
    std::function<void(UP_Socket)> new_connection;
public:
    Acceptor(EventLoop *loop, const uint16_t port, const std::string &ip);
    ~Acceptor() {}

    void NewConnection();
    void SetNewConnection(std::function<void(UP_Socket)> fn);
};

#endif // _ACCEPTOR_H_
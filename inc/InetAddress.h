#ifndef _INETADDRESS_H_
#define _INETADDRESS_H_

#include "../logger/Logger.h"

#include <string.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>


class InetAddress
{
public:
    struct sockaddr_in addr_;
    socklen_t addr_len_;
public:
    InetAddress();
    InetAddress(const uint16_t port, const std::string &ip);
    ~InetAddress() {}
    std::string GetIp() const;
    uint16_t GetPort() const;
};

#endif // _INETADDRESS_H_
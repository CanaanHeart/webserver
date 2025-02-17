#include "InetAddress.h"

using namespace std;

InetAddress::InetAddress(): addr_len_(sizeof(addr_))
{
    memset(&addr_, 0, sizeof(addr_));
}

InetAddress::InetAddress(const uint16_t port, const string &ip)
{
    memset(&addr_, 0, sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr);

    addr_len_ = sizeof(addr_);
}

string InetAddress::GetIp() const
{
    char buf[INET_ADDRSTRLEN];
    const char *res = inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, INET_ADDRSTRLEN);
    return string(res);
}

uint16_t InetAddress::GetPort() const
{
    return ntohs(addr_.sin_port);
}

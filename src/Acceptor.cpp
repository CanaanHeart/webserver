#include "Acceptor.h"

using namespace std;

Acceptor::Acceptor(EventLoop *loop, const uint16_t port, const string &ip):
            loop_(loop), serv_sock_(), serv_ch_(loop_, serv_sock_.GetFd())
{
    // serv_sock_.SetNonBlock();
    serv_sock_.SetReuseAddr();
    serv_sock_.SetReusePort();
    serv_sock_.SetNoDelay();

    InetAddress serv_addr(port, ip);

    serv_sock_.Bind(serv_addr);
    serv_sock_.Listen();

    serv_ch_.SetReadCB(std::bind(&Acceptor::NewConnection, this));
    serv_ch_.EnableRead();
}

void Acceptor::NewConnection()
{
    InetAddress cli_addr;

    int cfd = serv_sock_.Accept(cli_addr);

    UP_Socket cli_sock = make_unique<Socket>(cfd);
    cli_sock->SetNonBlock();
    cli_sock->SetPortIp(cli_addr.GetPort(), cli_addr.GetIp());
    
    new_connection(std::move(cli_sock));
}

void Acceptor::SetNewConnection(std::function<void(UP_Socket)> fn)
{
    new_connection = fn;
}
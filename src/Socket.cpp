#include "Socket.h"

using namespace std;

Socket::Socket() : fd_(-1)
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_ == -1){
        ERROR("socket error!");
    }
}

Socket::Socket(int fd) : fd_(fd)
{
    if(fd_ == -1){
        ERROR("socket error!");
    }
}

Socket::~Socket()
{
    close(fd_);
}

void Socket::Bind(const InetAddress &addr)
{
    int k = bind(fd_, reinterpret_cast<const struct sockaddr *>(&(addr.addr_)), addr.addr_len_);
    if(k == -1){
        ERROR("bind error!");
    }

    SetPortIp(addr.GetPort(), addr.GetIp());
}

void Socket::Listen(int n)
{
    int k = listen(fd_, n);
    if(k == -1){
        ERROR("listen error!");
    }
}

int Socket::SetNonBlock()
{
    int old_option = fcntl(fd_, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    int ret = fcntl(fd_, F_SETFL, new_option);
    if(ret == -1){
        ERROR("set nonblock error!");
    }

    return old_option;
}

bool Socket::IsNonBlock()
{
    return ((fcntl(fd_, F_GETFL) & O_NONBLOCK) != 0);
}

int Socket::Accept(InetAddress &addr)
{
    int cfd = -1;

    if(IsNonBlock()){
        while(1){
            cfd = accept(fd_, reinterpret_cast<struct sockaddr *>(&(addr.addr_)), &(addr.addr_len_));
            if(cfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                continue;
            if(cfd == -1){
                ERROR("accept error!");
            }
            else
                break;
        }
    }
    else{
        cfd = accept(fd_, reinterpret_cast<struct sockaddr *>(&(addr.addr_)), &(addr.addr_len_));
        if(cfd == -1){
            ERROR("accept error!");
        }
    }

    return cfd;
}

void Socket::Connect(const InetAddress &addr)
{
    if(IsNonBlock()){
        while(1){
            int ret = connect(fd_, reinterpret_cast<const struct sockaddr *>(&(addr.addr_)), addr.addr_len_);
            if(ret == 0)
                break;
            if((ret == -1) && (errno = EINPROGRESS))
                continue;
            if(ret == -1){
                ERROR("connect error!");
            }
        }
    }
    else{
        int ret = connect(fd_, reinterpret_cast<const struct sockaddr *>(&(addr.addr_)), addr.addr_len_);
        if(ret == -1){
            ERROR("connect error!");
        }
    }
}

void Socket::SetPortIp(const uint16_t &port, const std::string &ip)
{
    port_ = port;
    ip_ = ip;
}

void Socket::SetReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));
    if(ret == -1){
        INFO("reuse addr fail!");
    }
}

void Socket::SetReusePort(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
    if(ret == -1){
        INFO("reuse port fail!");
    }
}

void Socket::SetNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));
    if(ret == -1){
        INFO("set nodelay fail!");
    }
}

void Socket::SetKeepAlive(bool on)
{   
    int opt = on ? 1 : 0;
    int ret = setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
    if(ret == -1){
        INFO("set keep alive fail!");
    }
}

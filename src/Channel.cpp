#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"

using namespace std;

Channel::~Channel()
{
    DisableAll();
}

int Channel::GetFd()
{
    return fd_;
}

void Channel::UseET()
{
    listen_ev_ |= EPOLLET;
    use_et_ = true;
    loop_->UpdateCh(this);
}
void Channel::EnableRead()
{
    listen_ev_ |= EPOLLIN;
    loop_->UpdateCh(this);
}

void Channel::DisableRead()
{
    listen_ev_ &= ~EPOLLIN;
    loop_->UpdateCh(this);
}

void Channel::EnableWrite()
{
    listen_ev_ |= EPOLLOUT;
    loop_->UpdateCh(this);
}

void Channel::DisableWrite()
{
    listen_ev_ &= ~EPOLLOUT;
    loop_->UpdateCh(this);
}

void Channel::DisableAll()
{
    listen_ev_ = 0;
    loop_->DeleteCh(this);
}

bool Channel::InEpoll()
{
    return in_epoll_;
}

void Channel::SetInEpoll(bool in_epoll)
{
    in_epoll_ = in_epoll;
}

uint32_t Channel::GetListenEv()
{
    return listen_ev_;
}

uint32_t Channel::GetReadyEv()
{
    return ready_ev_;
}

void Channel::SetReadyEv(uint32_t ev)
{
    ready_ev_ = ev;
}

void Channel::Handle()
{
    if(GetReadyEv() & EPOLLRDHUP){
        // 检测到对端断开连接
        // DEBUG("EPOLLRDHUP");
        DisableAll();
        close_cb_();
    }
    else if(GetReadyEv() & (EPOLLIN | EPOLLPRI)){
        // DEBUG("EPOLLIN");
        read_cb_();
    }
    else if(GetReadyEv() & EPOLLOUT){
        // DEBUG("EPOLLOUT");
        write_cb_();
    }
    else{
        DisableAll();
        error_cb_();
    }
}

void Channel::SetReadCB(std::function<void()> fn)
{
    read_cb_ = fn;
}

void Channel::SetCloseCB(std::function<void()> fn)
{
    close_cb_ = fn;
}

void Channel::SetErrorCB(std::function<void()> fn)
{
    error_cb_ = fn;
}

void Channel::SetWriteCB(std::function<void()> fn)
{
    write_cb_ = fn;
}
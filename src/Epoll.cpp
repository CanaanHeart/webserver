#include "Epoll.h"
#include "Channel.h"

using namespace std;

Epoll::Epoll():epfd_(-1)
{
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if(epfd_ == -1){
        ERROR("epoll create error!");
    }
    
    memset(events_, 0, sizeof(events_));
}

Epoll::~Epoll()
{
    if(epfd_ != -1)
        close(epfd_);
    epfd_ = -1;
}

void Epoll::UpdateCh(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->GetListenEv();

    int ret = 0;

    if(ch->InEpoll()){
        ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, ch->GetFd(), &ev);
        if(ret == -1){
            ERROR("epoll add error!");
        }
    }
    else{
        ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->GetFd(), &ev);
        if(ret == -1){
            ERROR("epoll add error!");
        }
        ch->SetInEpoll();
    }
}

void Epoll::DeleteCh(Channel *ch)
{
    if(ch->InEpoll()){
        int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->GetFd(), 0);
        if(ret == -1){
            ERROR("epoll del error!");
        }
        ch->SetInEpoll(false);
    }
}

vector<Channel*> Epoll::Poll(int timeout)
{
    vector<Channel*> chs;

    memset(events_, 0, sizeof(events_));

    int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    if(nfds == -1){
        ERROR("epoll wait error!");
    }
    else if(nfds == 0)
        return chs;
    for(int i = 0; i < nfds; ++i){
        Channel *ch = static_cast<Channel*>(events_[i].data.ptr);
        ch->SetReadyEv(events_[i].events);
        chs.push_back(ch);
    }

    return chs;
}
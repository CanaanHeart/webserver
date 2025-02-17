#ifndef _EPOLL_H_
#define _EPOLL_H_

#include "../logger/Logger.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define MAX_EVENTS 1024

class Channel;
class Epoll;

using UP_Epoll = std::unique_ptr<Epoll>;

class Epoll{
private:
    int epfd_;
    epoll_event events_[MAX_EVENTS];
public:
    Epoll();
    ~Epoll();
    void UpdateCh(Channel *ch);
    void DeleteCh(Channel *ch);
    std::vector<Channel*> Poll(int timeout);
};

#endif // _EPOLL_H_
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "../logger/Logger.h"
#include "InetAddress.h"
#include "Socket.h"

#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <iostream>
#include <string>
#include <functional>

class Epoll;
class EventLoop;

class Channel{
private:
    EventLoop *loop_;
    int fd_ = -1;
    bool in_epoll_ = false; 
    bool use_et_ = false; 
    uint32_t listen_ev_ = 0;   
    uint32_t ready_ev_ = 0;   
    std::function<void()> read_cb_;    
    std::function<void()> close_cb_;    
    std::function<void()> error_cb_;    
    std::function<void()> write_cb_;
public:
    Channel(EventLoop *loop, int fd): loop_(loop), fd_(fd) {}
    ~Channel();

    int GetFd();
    void UseET();
    void EnableRead();  
    void DisableRead(); 
    void EnableWrite(); 
    void DisableWrite();    
    void DisableAll();  
    bool InEpoll(); 
    void SetInEpoll(bool in_epoll = true);  
    uint32_t GetListenEv(); 
    uint32_t GetReadyEv();  
    void SetReadyEv(uint32_t ev);
    void Handle();

    void SetReadCB(std::function<void()> fn); 
    void SetCloseCB(std::function<void()> fn);
    void SetErrorCB(std::function<void()> fn);
    void SetWriteCB(std::function<void()> fn);
};

#endif // _CHANNEL_H_
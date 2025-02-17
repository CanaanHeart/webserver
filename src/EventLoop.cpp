#include "EventLoop.h"
#include "Channel.h"
#include "Connection.h"

using namespace std;

int CreateTimer(int sec)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if(tfd == -1){
        ERROR("timerfd create error!");
    }
    itimerspec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.it_value.tv_sec = sec;
    timeout.it_value.tv_nsec = 0;
    if(timerfd_settime(tfd, 0, &timeout, 0) == -1){
        ERROR("timerfd set error!");
    }
    return tfd;
}

EventLoop::EventLoop(bool is_main_loop, int tvl, int tou): 
            is_main_loop_(is_main_loop), tvl_(tvl), tou_(tou), quit_(false)
{
    ep_ = make_unique<Epoll>();
    evfd_ = eventfd(0, EFD_NONBLOCK);
    if(evfd_ == -1){
        ERROR("eventfd create error!");
    }
    wake_ch_ = make_unique<Channel>(this, evfd_);
    wake_ch_->SetReadCB(std::bind(&EventLoop::WakeUpHandle, this));
    wake_ch_->EnableRead();

    tmfd_ = CreateTimer(tou_);

    time_ch_ = make_unique<Channel>(this, tmfd_);
    time_ch_->SetReadCB(std::bind(&EventLoop::TimerHandle, this));
    time_ch_->EnableRead();
}

void EventLoop::UpdateCh(Channel *ch)
{
    ep_->UpdateCh(ch);
}

void EventLoop::DeleteCh(Channel *ch)
{
    ep_->DeleteCh(ch);
}

void EventLoop::Loop()
{
    tid_ = syscall(SYS_gettid);

    int timeout = 0;

    while(!quit_){
        vector<Channel*> chs = ep_->Poll(timeout);

        // if(chs.size() == 0)
        //     m_epoll_timeout_cb(this);

        for(auto &ch: chs)
            ch->Handle();
    }
}

void EventLoop::Stop()
{
    quit_ = true;
    // DEBUG("eventloop stop!");
    WakeUp();
}

void EventLoop::SetConnTimeoutCB(std::function<void(SP_Connection)> fn)
{
    conn_timeout_cb_ = fn;
}

bool EventLoop::IsLoopThread()
{
    return tid_ == syscall(SYS_gettid);
}

void EventLoop::AddTask(std::function<void()> fn)
{
    {
        lock_guard<mutex> lock(mtx_qu_);
        qu_.push(fn);
    }
    WakeUp();
}

void EventLoop::WakeUp()
{
    uint64_t val = 1;
    int n = write(evfd_, &val, sizeof(val));
    if(n == -1){
        ERROR("write eventfd error!");
    }
}

void EventLoop::WakeUpHandle()
{
    uint64_t val;
    int n = read(evfd_, &val, sizeof(val));
    if(n == -1){
        ERROR("read eventfd error!");
    }

    function<void()> fn;

    lock_guard<mutex> lock(mtx_qu_);

    while(qu_.size() > 0){
        fn = std::move(qu_.front());
        qu_.pop();
        fn();
    }
}

void EventLoop::TimerHandle()
{
    itimerspec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.it_value.tv_sec = tvl_;
    timeout.it_value.tv_nsec = 0;
    if(timerfd_settime(tmfd_, 0, &timeout, 0) == -1){
        ERROR("timerfd set error!");
    }
    if(is_main_loop_){
        // DEBUG("main_loop闹钟到时!");
    }
    else{
        // DEBUG("sub_loop闹钟到时!");
        time_t now = time(0);
        for(auto it = conns_.begin(); it != conns_.end();){
            if(it->second->IsTimeout(now, tou_)){                
                auto tmp = it;
                {
                    // lock_guard<mutex> lock(m_mtx_conns);
                    it = conns_.erase(it);
                }
                conn_timeout_cb_(tmp->second);
            }
            else
                ++it;
        }
    }
}

void EventLoop::AddConnectionMap(SP_Connection conn)
{
    lock_guard<mutex> lock(mtx_conns_);
    conns_[conn->GetFd()] = conn;
}

void EventLoop::ClearConnectionFromMap(int fd)
{
    lock_guard<mutex> lock(mtx_conns_);
    conns_.erase(fd);
}
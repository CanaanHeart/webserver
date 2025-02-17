#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include "Epoll.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include <atomic>

class Channel;
class Connection;

using SP_Connection = std::shared_ptr<Connection>;
using UP_Channel = std::unique_ptr<Channel>;

int CreateTimer(int sec = 5);

class EventLoop{
private:
    UP_Epoll ep_ = nullptr;
    bool is_main_loop_;                     // 是否为主事件循环
    int tvl_;                               // 闹钟时间间隔
    int tou_;                               // Connection超时时间
    std::atomic_bool quit_;                 // true，退出事件循环
    pid_t tid_;                             // 事件循环所在线程id
    std::queue<std::function<void()>> qu_;
    std::mutex mtx_qu_;                     // 任务队列锁
    std::mutex mtx_conns_;                  // 保护conns_的锁
    int evfd_;
    UP_Channel wake_ch_;                    // 唤醒ch
    int tmfd_; 
    UP_Channel time_ch_;                    // 定时器ch
    std::map<int, SP_Connection> conns_;

    std::function<void(SP_Connection)> conn_timeout_cb_;
    
public:
    EventLoop(bool is_main_loop, int tvl = 10, int tou = 20);
    ~EventLoop() = default;
    void UpdateCh(Channel *ch);
    void DeleteCh(Channel *ch);
    void Loop();
    void Stop();
    void SetConnTimeoutCB(std::function<void(SP_Connection)> fn);
    bool IsLoopThread();
    void AddTask(std::function<void()> fn);
    void WakeUp();
    void WakeUpHandle();    // 事件循环被唤醒后的函数
    void TimerHandle();     // 定时器回调函数
    void AddConnectionMap(SP_Connection conn);
    void ClearConnectionFromMap(int fd);
};

#endif // _EVENTLOOP_H_
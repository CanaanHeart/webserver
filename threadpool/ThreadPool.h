#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "Thread.h"
#include "ThreadLoadConfig.h"

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <unordered_map>
#include <future>

enum class ThreadPoolMode
{
    FIXED,          // 固定数量的线程池
    CACHED          // 线程数量动态调整的线程池
};

enum class ThreadPoolType
{
    IO,
    WORK
};

using Task = std::function<void()>;

class ThreadPool
{
private:
    ThreadLoadConfig tlc_;
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;  // 线程列表
    size_t max_thread_nums_;                        // 线程数量上限
    size_t init_thread_nums_;                       // 初始线程数量
    std::atomic_uint idle_thread_nums_;             // 空闲线程数量

    std::queue<Task> task_qu_;
    std::atomic_uint task_nums_;                    // 任务数量
    size_t max_task_nums_;                          // 任务数量上限
    std::mutex task_qu_mtx_;                        // 任务队列锁
    std::condition_variable not_full_;              // 任务队列不满
    std::condition_variable not_empty_;             // 任务队列不空
    ThreadPoolMode mode_;                           // 线程池工作模式
    ThreadPoolType type_;                           // 线程池类型，分为I/0线程池和WORK线程池

    std::atomic_bool stop_;                         // 线程池是否停止
    std::condition_variable exit_;                  // 等待线程资源全部回收
public:
    ThreadPool(const std::string &file);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ~ThreadPool();

    void Start();
    size_t GetThreadNums();                         // 获取当前线程数量
    size_t GetTaskNums();                           // 获取任务数量

    template<typename Func, typename... Args>
    auto AddTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

private:
    void ThreadFunc(int trhread_id);
};

template<typename Func, typename... Args>
auto ThreadPool::AddTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
{
    using RType = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<RType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );

    std::future<RType> result = task->get_future();

    std::unique_lock<std::mutex> ulock(task_qu_mtx_);

    // 如果等待1s，任务队列仍满，则直接返回
    if(!not_full_.wait_for(ulock, std::chrono::seconds(1),[this](){return task_qu_.size() < max_task_nums_;})){
        WARN("task queue is full, add task fail!");
        auto task = std::make_shared<std::packaged_task<RType()>>(
            []()->RType{return RType();}
        );
        (*task)();
        return task->get_future();
    }

    task_qu_.emplace([task](){ (*task)();});
    ++task_nums_;
    
    // CACHED模式，适合小而快的任务，需要根据任务数量和空闲线程的数量，判断是否创建新线程
    if(mode_ == ThreadPoolMode::CACHED && task_nums_ > idle_thread_nums_ && threads_.size() < max_thread_nums_){
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        int thread_id = ptr->GetId();
        threads_.emplace(thread_id, std::move(ptr));

        DEBUG("创建新线程!");

        threads_[thread_id]->Start();
        // ++curr_thread_nums_;
        ++idle_thread_nums_;
    }

    not_empty_.notify_all();

    return result;
}

#endif /* __THREADPOOL_H__ */
#include "ThreadPool.h"

using namespace std;

#define MAX_IDLE_TIME 10

ThreadPool::ThreadPool(const string &file)
{
    tlc_.LoadConfigFile(file);
    max_thread_nums_ = static_cast<size_t>(stoi(tlc_.GetValue("max_thread_nums")));
    init_thread_nums_ = static_cast<size_t>(stoi(tlc_.GetValue("init_thread_nums")));
    // curr_thread_nums_ = 0;
    idle_thread_nums_ = 0;
    task_nums_ = 0;
    max_task_nums_ = static_cast<size_t>(stoi(tlc_.GetValue("max_task_nums")));

    string mode = tlc_.GetValue("mode");
    if(mode == "FIXED")
        mode_ = ThreadPoolMode::FIXED;
    else
        mode_ = ThreadPoolMode::CACHED;

    string type = tlc_.GetValue("type");
    if(type == "IO")
        type_ = ThreadPoolType::IO;
    else
        type_ = ThreadPoolType::WORK;
}

ThreadPool::~ThreadPool()
{
    stop_ = true;
    unique_lock<mutex> ulock(task_qu_mtx_);
    not_empty_.notify_all();
    exit_.wait(ulock, [this](){return threads_.empty();});
}

void ThreadPool::Start()
{
    stop_ = false;

    // 创建线程对象
    for(size_t i = 0; i < init_thread_nums_; ++i){
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        threads_.emplace(ptr->GetId(), std::move(ptr));
    }
    
    // 启动线程
    for(auto it = threads_.begin(); it != threads_.end(); ++it){
        it->second->Start();
        ++idle_thread_nums_;
    }
}

size_t ThreadPool::GetThreadNums()
{
    return threads_.size();
}

size_t ThreadPool::GetTaskNums()
{
    unique_lock<mutex> ulock(task_qu_mtx_);
    return task_qu_.size();
}

void ThreadPool::ThreadFunc(int thread_id)
{
    auto last_time = chrono::high_resolution_clock().now();
    while(true){
        Task task;

        {
            unique_lock<mutex> ulock(task_qu_mtx_);
            string type;
            if(type_ == ThreadPoolType::IO){
                type = "IO";
            }
            else{
                type = "WORK";
            }

            // DEBUG(type, " tid: ", thread_id, " 尝试获取任务!");

            // 超过初始线程数量的线程要进行回收
            
            while(task_qu_.size() == 0){
                if(stop_){
                    threads_.erase(thread_id);
                    // DEBUG(type, " tid: ", thread_id, " exit!");
                    exit_.notify_all();
                    return;
                }

                if(mode_ == ThreadPoolMode::CACHED){
                // 超时返回
                    if(std::cv_status::timeout == not_empty_.wait_for(ulock, chrono::seconds(1))){
                        auto now = chrono::high_resolution_clock().now();
                        auto dur = chrono::duration_cast<chrono::seconds>(now - last_time);
                        if(dur.count() >= MAX_IDLE_TIME && threads_.size() > init_thread_nums_){
                            threads_.erase(thread_id);
                            --idle_thread_nums_;
                            return;
                        }
                    }
                }
                else{
                    not_empty_.wait(ulock);
                }

                if(stop_){
                    threads_.erase(thread_id);
                    // DEBUG("stop! ", type, " tid: ", thread_id, " exit!");
                    exit_.notify_all();
                    return;
                }
            }

            task = task_qu_.front();
            task_qu_.pop();
            --task_nums_;
            --idle_thread_nums_;

            // DEBUG(type, " tid: ", thread_id, " 获取任务成功!");

            if(!task_qu_.empty())
                not_empty_.notify_all();
            not_full_.notify_all();
        }

        // DEBUG("task run!");

        if(task != nullptr)
            task();

        // DEBUG("task over!");
        
        ++idle_thread_nums_;
        last_time = chrono::high_resolution_clock().now();
    }
}
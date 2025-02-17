#ifndef __THREAD_H__
#define __THREAD_H__

#include <iostream>
#include <thread>
#include <functional>

using ThreadFunc = std::function<void(int)>;

class Thread
{
private:
    ThreadFunc func_;
    int thread_id_;
    static int generate_id_;
public:
    Thread(ThreadFunc func);
    ~Thread() = default;
    void Start();
    int GetId() const;
};

#endif /* __THREAD_H__ */
#include "Thread.h"

using namespace std;

int Thread::generate_id_ = 0;

Thread::Thread(ThreadFunc func):func_(func), thread_id_(generate_id_++)
{
    
}

void Thread::Start()
{
    thread th(func_, thread_id_);
    th.detach();
}

int Thread::GetId() const
{
    return thread_id_;
}
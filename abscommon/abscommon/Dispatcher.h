#pragma once

#include "Task.h"
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace Asynch {

class Dispatcher
{
public:
    Dispatcher() :
        state_(State::Terminated),
        utilization_(0)
    {
        tasks_.clear();
    }
    ~Dispatcher() = default;

    void Start();
    void Stop();
    void Add(Task* task, bool front = false);
    /// CPU Utilization in % something between 0..100
    uint32_t GetUtilization() const
    {
        return utilization_;
    }

    bool IsDispatcherThread() const { return (state_ == State::Running) ? thread_.get_id() == std::this_thread::get_id() : false; }

    enum class State
    {
        Running,
        Terminated
    };
private:
    std::mutex lock_;
    std::list<Task*> tasks_;
    State state_;
    std::thread thread_;
    std::condition_variable signal_;
    uint32_t utilization_;
    void DispatcherThread();
};

}

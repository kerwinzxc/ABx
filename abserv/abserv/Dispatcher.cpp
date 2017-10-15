#include "stdafx.h"
#include "Dispatcher.h"
#include "Logger.h"
#include "OutputMessage.h"

Dispatcher Dispatcher::Instance;

void Dispatcher::Start()
{
    state_ = State::Running;
    thread_ = std::thread(DispatcherThread, (void*)this);
}

void Dispatcher::Stop()
{
    lock_.lock();
    state_ = State::Closing;
    lock_.unlock();
}

void Dispatcher::Terminate()
{
    lock_.lock();
    state_ = State::Terminated;
    lock_.unlock();
}

void Dispatcher::Add(Task* task, bool front /* = false */)
{
    bool doSignal = false;
    lock_.lock();

    if (state_ == State::Running)
    {
        doSignal = tasks_.empty();

        if (!front)
            tasks_.push_back(task);
        else
            tasks_.push_front(task);
    }

    lock_.unlock();

    if (doSignal)
        signal_.notify_one();
}

void Dispatcher::DispatcherThread(void* p)
{
#ifdef DEBUG_DISPATTCHER
    LOG_DEBUG << "Dispatcher threat started" << std::endl;
#endif

    Dispatcher* dispatcher = (Dispatcher*)p;

    OutputMessagePool* outputPool;

    // NOTE: second argument defer_lock is to prevent from immediate locking
    std::unique_lock<std::mutex> taskLockUnique(dispatcher->lock_, std::defer_lock);

    while (dispatcher->state_ != State::Terminated)
    {
        Task* task = nullptr;

        taskLockUnique.lock();

        if (dispatcher->tasks_.empty())
        {
            // List is empty, wait for signal
            dispatcher->signal_.wait(taskLockUnique);
#ifdef DEBUG_DISPATTCHER
            LOG_DEBUG << "Waiting for task" << std::endl;
#endif
        }

#ifdef DEBUG_DISPATTCHER
        LOG_DEBUG << "Dispatcher signaled" << std::endl;
#endif

        if (!dispatcher->tasks_.empty() && (dispatcher->state_ != State::Terminated))
        {
            // Take first task
            task = dispatcher->tasks_.front();
            dispatcher->tasks_.pop_front();
        }

        taskLockUnique.unlock();

        // Execute the task
        if (task)
        {
            if (!task->IsExpired())
            {
                OutputMessagePool::Instance()->StartExecutionFrame();
                (*task)();

                outputPool = OutputMessagePool::Instance();
                if (outputPool)
                    outputPool->SendAll();
            }

            delete task;

#ifdef DEBUG_DISPATTCHER
            LOG_DEBUG << "Executing task" << std::endl;
#endif
        }
    }
#ifdef DEBUG_DISPATTCHER
    LOG_DEBUG << "Dispatcher threat stopped" << std::endl;
#endif
}

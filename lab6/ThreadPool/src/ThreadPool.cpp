#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threads) : m_tasks(128)
{
    if (threads == 0)
    {
        throw std::invalid_argument("Thread count must be positive");
    }

    for (size_t i = 0; i < threads; ++i)
    {
        m_workers.emplace_back(&ThreadPool::WorkerLoop, this);
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_cv.notify_all();

    for (auto& thread: m_workers)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    void* taskPtr = nullptr;
    while (m_tasks.pop(taskPtr))
    {
        delete static_cast<std::function<void()>*>(taskPtr);
    }
}

void ThreadPool::WorkerLoop()
{
    while (!m_stop)
    {
        void* taskPtr = nullptr;
        if (m_tasks.pop(taskPtr))
        {
            //Если задача есть - выполняет ее и уменьшает счетчик
            std::function<void()>* task = static_cast<std::function<void()>*>(taskPtr);
            (*task)();
            delete task;
            m_taskCount--;
        } else
        {
            //Если очередь пуста - ждет
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait_for(lock, std::chrono::milliseconds(1));
        }
    }
}
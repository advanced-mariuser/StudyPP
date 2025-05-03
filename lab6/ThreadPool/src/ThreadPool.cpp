#include "ThreadPool.h"
#include <stdexcept>

ThreadPool::ThreadPool(size_t threads) : tasks(threads * 2)
{
    if (threads == 0)
    {
        throw std::invalid_argument("Thread count must be positive");
    }

    for (size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back([this]
                             {
                                 while (true)
                                 {
                                     std::function<void()> task;

                                     {
                                         std::unique_lock<std::mutex> lock(cv_mutex);
                                         cv.wait(lock, [this]
                                         {
                                             return stop.load() || !tasks.empty();
                                         });

                                         if (stop && tasks.empty())
                                         {
                                             return;
                                         }

                                         if (tasks.pop(task))
                                         {
                                             taskCount--;
                                         } else
                                         {
                                             continue;
                                         }
                                     }

                                     task();
                                 }
                             });
    }
}

ThreadPool::~ThreadPool()
{
    stop.store(true);
    cv.notify_all();
    for (auto& worker: workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&& ... args)
-> std::future<typename std::invoke_result_t<F, Args...>>
{

    using return_type = typename std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    if (stop.load())
    {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }

    if (tasks.push([task]()
                   { (*task)(); }))
    {
        taskCount++;
        cv.notify_one();
    } else
    {
        throw std::runtime_error("Failed to enqueue task");
    }

    return res;
}
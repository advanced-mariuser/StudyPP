#pragma once

#include <boost/lockfree/queue.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <future>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

class ThreadPool
{
public:
    explicit ThreadPool(size_t threads);

    ~ThreadPool();

    template<class F, class... Args>
    auto Enqueue(F&& f, Args&& ... args)
    -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        //Утечка памяти
        //Перехватить и сделать delete
        auto wrapper = new std::function<void()>([task]()
                                                 { (*task)(); });

        while (!m_tasks.push(wrapper))
        {
            //Текущий поток немедленно приостанавливается
            //Планировщик ОС выбирает другой поток для выполнения
            //Позже этот поток будет возобновлен согласно политике планирования
            std::this_thread::yield();
        }

        //Увеличивает счетчик задач и уведомляет один поток
        m_taskCount++;
        m_cv.notify_one();
        return task->get_future();
    }

    size_t TasksPending() const { return m_taskCount.load(); }

private:
    void WorkerLoop();

    //Рабочие потоки
    std::vector<std::thread> m_workers;
    //Очередь задач
    boost::lockfree::queue<void*> m_tasks;
    std::atomic<bool> m_stop{false};
    std::atomic<size_t> m_taskCount{0};
    std::condition_variable m_cv;
    std::mutex m_mutex;
};
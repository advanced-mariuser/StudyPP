#pragma once

#include "LockFreeQueue.h"
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <future>

class ThreadPool
{
public:
    explicit ThreadPool(size_t threads);

    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&& ... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>;

    size_t size() const { return workers.size(); }

    size_t tasksPending() const { return taskCount; }

private:
    std::vector<std::thread> workers;
    LockFreeQueue <std::function<void()>> tasks;
    std::atomic<bool> stop{false};
    std::atomic<size_t> taskCount{0};
    std::condition_variable cv;
    std::mutex cv_mutex;
};
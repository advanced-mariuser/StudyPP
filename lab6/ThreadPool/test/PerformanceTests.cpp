#include "catch2/catch_test_macros.hpp"
#include "../src/ThreadPool.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <chrono>
#include <vector>
#include <atomic>
#include <iostream>
#include <mutex>

using namespace std::chrono;

template<typename Pool>
void run_benchmark(const std::string& name, Pool& pool, int tasks)
{
    std::atomic<int> counter{0};
    auto start = high_resolution_clock::now();

    for (int i = 0; i < tasks; ++i)
    {
        pool.enqueue([&]
                     { counter++; });
    }

    // Wait for completion
    while (counter < tasks)
    {
        std::this_thread::yield();
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    std::cout << name << " completed " << tasks
              << " tasks in " << duration << "ms\n";
}

TEST_CASE("Performance comparison")
{
    constexpr int tasks = 100000;
    constexpr int min_threads = 1;
    const int max_threads = 2 * std::thread::hardware_concurrency();

    std::cout << "Performance comparison (" << tasks << " tasks)\n";
    std::cout << "Threads,LockFree,LockBased,Asio\n";

    for (int threads = min_threads; threads <= max_threads; ++threads)
    {
        // Lock-free ThreadPool
        auto lf_start = high_resolution_clock::now();
        {
            ThreadPool lf_pool(threads);
            run_benchmark("LockFree", lf_pool, tasks);
        }
        auto lf_duration = duration_cast<milliseconds>(
                high_resolution_clock::now() - lf_start).count();

        // Lock-based ThreadPool (similar implementation but with mutex)
        auto lb_start = high_resolution_clock::now();
        {
            // LockBasedThreadPool lb_pool(threads);
            // run_benchmark("LockBased", lb_pool, tasks);
        }
        auto lb_duration = 0; // Replace with actual measurement

        // Boost.Asio thread_pool
        auto asio_start = high_resolution_clock::now();
        {
            boost::asio::thread_pool asio_pool(threads);
            for (int i = 0; i < tasks; ++i)
            {
                boost::asio::post(asio_pool, []
                {});
            }
            asio_pool.join();
        }
        auto asio_duration = duration_cast<milliseconds>(
                high_resolution_clock::now() - asio_start).count();

        std::cout << threads << "," << lf_duration << ","
                  << lb_duration << "," << asio_duration << "\n";
    }
}
#include "catch2/catch_test_macros.hpp"
#include "../src/ThreadPool.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <chrono>
#include <atomic>
#include <iostream>
#include <mutex>
#include <fstream>

using namespace std::chrono;

void RunBenchmark(const std::string& name, int threadCount, int taskCount, std::ostream& csv)
{
    std::atomic<int> counter{0};

    // Lock-Free
    auto lfStart = high_resolution_clock::now();
    {
        ThreadPool pool(threadCount);
        for (int i = 0; i < taskCount; ++i)
        {
            //Везде увеличивать атомарную переменную
            pool.Enqueue([&]
                         { counter.fetch_add(1, std::memory_order_relaxed); });
        }

        while (pool.TasksPending() > 0)
        {
            std::this_thread::yield();
        }
    }
    auto lfDuration = duration_cast<milliseconds>(high_resolution_clock::now() - lfStart).count();

    // boost::asio
    auto asioStart = high_resolution_clock::now();
    {
        boost::asio::thread_pool asioPool(threadCount);
        for (int i = 0; i < taskCount; ++i)
        {
            boost::asio::post(asioPool, [&]
            {counter.fetch_add(1, std::memory_order_relaxed);});
        }
        asioPool.join();
    }
    auto asioDuration = duration_cast<milliseconds>(high_resolution_clock::now() - asioStart).count();

    csv << threadCount << "," << lfDuration << "," << asioDuration << "\n";
}

TEST_CASE("Performance CSV output")
{
    int taskCount = 1'600'000;
    const int maxThreads = 2 * static_cast<int>(std::thread::hardware_concurrency());

    std::ofstream csv("performance.csv");
    csv << "Threads,LockFree,BoostAsio\n";

    for (int t = 1; t <= maxThreads; ++t)
    {
        RunBenchmark("Benchmark", t, taskCount/t, csv);
    }

    std::cout << "CSV results written to performance.csv\n";
}
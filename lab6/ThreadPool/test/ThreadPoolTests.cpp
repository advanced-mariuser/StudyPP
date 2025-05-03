#include "catch2/catch_test_macros.hpp"
#include "../src/ThreadPool.h"
#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("ThreadPool basic functionality")
{
    ThreadPool pool(4);

    SECTION("Simple task execution")
    {
        auto future = pool.enqueue([]
                                   { return 42; });
        REQUIRE(future.get() == 42);
    }

    SECTION("Multiple tasks")
    {
        std::vector<std::future<int>> results;
        for (int i = 0; i < 10; ++i)
        {
            results.emplace_back(pool.enqueue([i]
                                              { return i * i; }));
        }

        for (int i = 0; i < 10; ++i)
        {
            REQUIRE(results[i].get() == i * i);
        }
    }

    SECTION("Exception handling")
    {
        auto future = pool.enqueue([]
                                   {
                                       throw std::runtime_error("test error");
                                       return 0;
                                   });

        REQUIRE_THROWS_AS(future.get(), std::runtime_error);
    }
}

TEST_CASE("ThreadPool edge cases")
{
    SECTION("Zero threads")
    {
        REQUIRE_THROWS_AS(ThreadPool(0), std::invalid_argument);
    }

    SECTION("Destruction with pending tasks")
    {
        std::atomic<int> counter{0};
        {
            ThreadPool pool(2);
            for (int i = 0; i < 100; ++i)
            {
                pool.enqueue([&]
                             {
                                 std::this_thread::sleep_for(10ms);
                                 counter++;
                             });
            }
        }
        REQUIRE(counter < 100);
    }
}

TEST_CASE("ThreadPool concurrency")
{
    constexpr int N = 1000;
    std::atomic<int> counter{0};

    ThreadPool pool(4);

    for (int i = 0; i < N; ++i)
    {
        pool.enqueue([&]
                     { counter++; });
    }

    // Wait for all tasks to complete
    while (pool.tasksPending() > 0)
    {
        std::this_thread::sleep_for(10ms);
    }

    REQUIRE(counter == N);
}
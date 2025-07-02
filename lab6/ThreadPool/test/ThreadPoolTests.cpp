#include "catch2/catch_test_macros.hpp"
#include "../src/ThreadPool.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

TEST_CASE("ThreadPool basic functionality")
{
    ThreadPool pool(4);

    SECTION("Single task")
    {
        auto future = pool.Enqueue([]
                                   { return 123; });
        REQUIRE(future.get() == 123);
    }

    SECTION("Multiple tasks")
    {
        std::vector<std::future<int>> results;
        results.reserve(10);
        for (int i = 0; i < 10; ++i)
        {
            results.push_back(pool.Enqueue([i]
                                           { return i * 2; }));
        }

        for (int i = 0; i < 10; ++i)
        {
            REQUIRE(results[i].get() == i * 2);
        }
    }

    SECTION("Exception")
    {
        auto future = pool.Enqueue([]
                                   { throw std::runtime_error("test"); });
        REQUIRE_THROWS_AS(future.get(), std::runtime_error);
    }
}

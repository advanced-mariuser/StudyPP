#include "catch2/catch_test_macros.hpp"
#include "../src/AtomicMax.h"
#include <thread>
#include <vector>
#include <limits>
#include <barrier>

TEST_CASE("AtomicMax basic functionality")
{
    SECTION("Integer types")
    {
        AtomicMax<int> maxInt(0);
        REQUIRE(maxInt.GetValue() == 0);

        maxInt.Update(10);
        REQUIRE(maxInt.GetValue() == 10);

        maxInt.Update(5);
        REQUIRE(maxInt.GetValue() == 10);

        maxInt.Update(15);
        REQUIRE(maxInt.GetValue() == 15);
    }

    SECTION("Floating point types")
    {
        AtomicMax<double> maxDouble(0.0);
        REQUIRE(maxDouble.GetValue() == 0.0);

        maxDouble.Update(3.14);
        REQUIRE(maxDouble.GetValue() == 3.14);

        maxDouble.Update(2.71);
        REQUIRE(maxDouble.GetValue() == 3.14);

        maxDouble.Update(4.25);
        REQUIRE(maxDouble.GetValue() == 4.25);
    }

    SECTION("Edge cases")
    {
        AtomicMax<int> maxInt(std::numeric_limits<int>::min());
        REQUIRE(maxInt.GetValue() == std::numeric_limits<int>::min());

        maxInt.Update(std::numeric_limits<int>::max());
        REQUIRE(maxInt.GetValue() == std::numeric_limits<int>::max());

        maxInt.Update(std::numeric_limits<int>::min());
        REQUIRE(maxInt.GetValue() == std::numeric_limits<int>::max());
    }
}

TEST_CASE("AtomicMax thread safety")
{
    constexpr int NumThreads = 10;
    constexpr int Iterations = 1000;

    AtomicMax<int> globalMax(0);
    std::barrier syncPoint(NumThreads);

    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < NumThreads; ++i)
        {
            //запустить одновременно потоки
            threads.emplace_back([&, i]()
                                 {
                                     int valueBase = (i + 1) * Iterations;
                                     syncPoint.arrive_and_wait();
                                     for (int j = 0; j < Iterations; ++j)
                                     {
                                         int value = valueBase + j;
                                         globalMax.Update(value);
                                     }
                                 });
        }
    }

    REQUIRE(globalMax.GetValue() == (NumThreads * Iterations) + Iterations - 1);
}

TEST_CASE("AtomicMax with different initial values")
{
    SECTION("Initial value is maximum")
    {
        AtomicMax<int> maxInt(100);
        maxInt.Update(50);
        REQUIRE(maxInt.GetValue() == 100);
    }

    SECTION("Initial value is minimum")
    {
        AtomicMax<int> maxInt(0);
        maxInt.Update(100);
        REQUIRE(maxInt.GetValue() == 100);
    }
}

TEST_CASE("AtomicMax type requirements")
{
    SECTION("Works with unsigned types")
    {
        AtomicMax<unsigned> maxUnsigned(0);
        maxUnsigned.Update(100);
        REQUIRE(maxUnsigned.GetValue() == 100);
    }

    SECTION("Works with large types")
    {
        AtomicMax<long long> maxLarge(0);
        maxLarge.Update(1'000'000'000'000LL);
        REQUIRE(maxLarge.GetValue() == 1'000'000'000'000LL);
    }
}
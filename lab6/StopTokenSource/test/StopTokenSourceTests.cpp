#include "catch2/catch_test_macros.hpp"
#include "../src/StopControl.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <barrier>

using namespace std::chrono_literals;

TEST_CASE("StopSource basic functionality")
{
    StopSource source;

    SECTION("Initially not stopped")
    {
        REQUIRE_FALSE(source.StopRequested());
    }

    SECTION("RequestStop sets flag")
    {
        source.RequestStop();
        REQUIRE(source.StopRequested());
    }

    SECTION("Multiple RequestStop calls are idempotent")
    {
        source.RequestStop();
        source.RequestStop();
        REQUIRE(source.StopRequested());
    }
}

TEST_CASE("StopToken basic functionality")
{
    StopSource source;
    StopToken token = source.GetToken();

    SECTION("Initially not stopped")
    {
        REQUIRE_FALSE(token.StopRequested());
    }

    SECTION("Reflects source state")
    {
        source.RequestStop();
        REQUIRE(token.StopRequested());
    }

    SECTION("Default constructed token is not stoppable")
    {
        StopToken emptyToken;
        REQUIRE_FALSE(emptyToken.StopRequested());
        REQUIRE_THROWS(emptyToken.WaitForStop());
    }
}

TEST_CASE("StopToken wait functionality")
{
    StopSource source;
    StopToken token = source.GetToken();

    SECTION("WaitForStop returns immediately if already stopped")
    {
        source.RequestStop();
        auto start = std::chrono::steady_clock::now();
        token.WaitForStop();
        auto duration = std::chrono::steady_clock::now() - start;
        REQUIRE(duration < 10ms);
    }

    SECTION("WaitForStop blocks until stop is requested")
    {
        std::atomic<bool> threadStarted{false};
        std::atomic<bool> waitFinished{false};

        {
            std::jthread t([&]
                           {
                               threadStarted = true;
                               token.WaitForStop();
                               waitFinished = true;
                           });

            REQUIRE_FALSE(waitFinished);

            source.RequestStop();
        }

        REQUIRE(waitFinished);
    }
}

TEST_CASE("Thread safety")
{
    constexpr int NumThreads = 10;

    SECTION("Multiple threads can check stop state")
    {
        StopSource source;
        StopToken token = source.GetToken();

        std::atomic<int> stopCount{0};
        std::barrier syncPoint(NumThreads);

        {
            std::vector<std::jthread> threads;
            for (int i = 0; i < NumThreads; ++i)
            {
                threads.emplace_back([&]
                                     {
                                         syncPoint.arrive_and_wait();
                                         if (token.StopRequested())
                                         {
                                             ++stopCount;
                                         }
                                     });
            }
        }

        REQUIRE(stopCount == 0);
    }

    SECTION("Multiple threads can request stop")
    {
        StopSource source;

        std::barrier syncPoint(NumThreads);

        {
            std::vector<std::jthread> threads;
            for (int i = 0; i < NumThreads; ++i)
            {
                threads.emplace_back([&]
                                     {
                                         syncPoint.arrive_and_wait();
                                         source.RequestStop();
                                     });
            }
        }

        REQUIRE(source.StopRequested());
    }
}

TEST_CASE("Token copy behavior")
{
    StopSource source;
    StopToken token1 = source.GetToken();

    SECTION("Copied token reflects same state")
    {
        StopToken token2 = token1;
        REQUIRE_FALSE(token2.StopRequested());

        source.RequestStop();
        REQUIRE(token1.StopRequested());
        REQUIRE(token2.StopRequested());
    }

    SECTION("Original and copy are independent")
    {
        StopToken token2 = token1;
        StopToken token3 = token2;

        source.RequestStop();
        REQUIRE(token1.StopRequested());
        REQUIRE(token2.StopRequested());
        REQUIRE(token3.StopRequested());
    }
}
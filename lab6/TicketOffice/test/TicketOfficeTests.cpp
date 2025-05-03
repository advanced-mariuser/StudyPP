#include "catch2/catch_test_macros.hpp"
#include "../src/TicketOffice.h"
#include <thread>
#include <vector>
#include <atomic>
#include <barrier>

TEST_CASE("TicketOffice single-threaded tests")
{
    SECTION("Initialization")
    {
        TicketOffice office(100);
        REQUIRE(office.GetTicketsLeft() == 100);

        REQUIRE_THROWS_AS(TicketOffice(-1), std::invalid_argument);
    }

    SECTION("Selling tickets")
    {
        TicketOffice office(100);

        REQUIRE(office.SellTickets(10) == 10);
        REQUIRE(office.GetTicketsLeft() == 90);

        REQUIRE(office.SellTickets(90) == 90);
        REQUIRE(office.GetTicketsLeft() == 0);
    }

    SECTION("Selling more tickets than available")
    {
        TicketOffice office(50);

        REQUIRE(office.SellTickets(60) == 50);
        REQUIRE(office.GetTicketsLeft() == 0);

        REQUIRE(office.SellTickets(10) == 0);
        REQUIRE(office.GetTicketsLeft() == 0);
    }

    SECTION("Invalid arguments")
    {
        TicketOffice office(100);

        REQUIRE_THROWS_AS(office.SellTickets(0), std::invalid_argument);
        REQUIRE_THROWS_AS(office.SellTickets(-1), std::invalid_argument);
    }
}

TEST_CASE("TicketOffice multi-threaded tests")
{
    constexpr int TotalTickets = 1000;
    constexpr int ThreadCount = 10;
    constexpr int TicketsPerThread = 150;

    TicketOffice office(TotalTickets);
    std::atomic<int> totalSold(0);
    std::barrier sync_point(ThreadCount);

    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < ThreadCount; ++i)
        {
            threads.emplace_back([&]()
                                 {
                                     sync_point.arrive_and_wait();
                                     int sold = office.SellTickets(TicketsPerThread);
                                     totalSold += sold;
                                 });
        }
    }

    REQUIRE(totalSold == TotalTickets);
    REQUIRE(office.GetTicketsLeft() == 0);

    REQUIRE(totalSold <= ThreadCount * TicketsPerThread);
}

TEST_CASE("TicketOffice precise multi-threaded test")
{
    constexpr int TotalTickets = 100;
    constexpr int ThreadCount = 4;

    TicketOffice office(TotalTickets);
    std::vector<int> soldByThread(ThreadCount, 0);
    std::barrier sync_point(ThreadCount);

    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < ThreadCount; ++i)
        {
            threads.emplace_back([&, i]()
                                 {
                                     sync_point.arrive_and_wait();
                                     soldByThread[i] = office.SellTickets(30);
                                 });
        }
    }

    int totalSold = 0;
    for (int sold: soldByThread)
    {
        totalSold += sold;
    }

    REQUIRE(totalSold == TotalTickets);
    REQUIRE(office.GetTicketsLeft() == 0);

    int count30 = 0;
    int count10 = 0;
    for (int sold: soldByThread)
    {
        if (sold == 30)
        {
            count30++;
        } else if (sold == 10)
        {
            count10++;
        } else
        {
            FAIL("Invalid sold count");
        }
    }

    REQUIRE(count30 == 3);
    REQUIRE(count10 == 1);
}
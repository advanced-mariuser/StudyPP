#include <iostream>
#include "src/StopControl.h"
#include <thread>
#include <chrono>
#include <tbb/concurrent_vector.h>

using namespace std::chrono_literals;

void WorkerThread(StopToken token)
{
    std::cout << "Worker started\n";

    // Проверяем флаг остановки периодически
    while (!token.StopRequested())
    {
        std::cout << "Working...\n";
        std::this_thread::sleep_for(500ms);
    }

    std::cout << "Worker stopped\n";
}

int main()
{
    try
    {
        StopSource source;
        std::jthread worker(WorkerThread, source.GetToken());

        // Даём потоку поработать 2 секунды
        std::this_thread::sleep_for(2s);

        // Запрашиваем остановку
        std::cout << "Requesting stop...\n";
        source.RequestStop();

        worker.join();
        std::cout << "Done\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
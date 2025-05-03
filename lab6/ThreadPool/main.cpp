#include "src/ThreadPool.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

int main()
{
    try
    {
        const size_t num_threads = std::thread::hardware_concurrency();
        ThreadPool pool(num_threads);

        std::vector<std::future<int>> results;

        for (int i = 0; i < 10; ++i)
        {
            results.emplace_back(pool.enqueue([i]
                                              {
                                                  std::this_thread::sleep_for(100ms);
                                                  return i * i;
                                              }));
        }

        for (auto& result: results)
        {
            std::cout << result.get() << ' ';
        }
        std::cout << std::endl;

    } catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
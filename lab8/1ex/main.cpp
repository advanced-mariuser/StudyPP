#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <execution>
#include "BitonicSorter.h"

void Generate(std::vector<int32_t>& data)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist;
    std::generate(data.begin(), data.end(), [&](){ return dist(gen); });
}

void RunGPU(std::vector<int32_t>& data)
{
    BitonicSorter sorter;
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        sorter.Sort(data);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "GPU Bitonic sort time: " << elapsed.count() << " seconds\n";

        if (std::is_sorted(data.begin(), data.end()))
        {
            std::cout << "GPU array is sorted correctly\n";
        } else
        {
            std::cout << "Error: GPU array is not sorted\n";
        }
    } catch (const std::exception& e)
    {
        std::cerr << "GPU sort failed: " << e.what() << "\n";
    }
}

void RunCPU(std::vector<int32_t>& cpuData)
{
    auto cpuStart = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, cpuData.begin(), cpuData.end());
    auto cpuEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> cpuElapsed = cpuEnd - cpuStart;
    std::cout << "CPU parallel std::sort time: " << cpuElapsed.count() << " seconds\n";

    if (std::is_sorted(cpuData.begin(), cpuData.end()))
    {
        std::cout << "CPU array is sorted correctly\n";
    } else
    {
        std::cout << "Error: CPU array is not sorted\n";
    }
}

//TODO: уметь отсортировать на листочке
int main()
{
    const size_t arraySize = 1 << 20; // ~1M elements
    std::vector<int32_t> data(arraySize);

    Generate(data);
    auto cpuData = data;

    RunGPU(data);
    RunCPU(cpuData);

    return EXIT_SUCCESS;
}


#pragma once

#include <vector>
#include <memory>
#include "GpuHelper.h"

class BitonicSorter
{
public:
    BitonicSorter();

    void Sort(std::vector<int32_t>& data);

private:
    void Initialize();

    void RunBitonicSort(cl_mem buffer, size_t size);

    std::unique_ptr<GpuHelper> m_gpuHelper;
    cl_program m_program;
    cl_kernel m_bitonicSortKernel;
    bool m_initialized = false;
};
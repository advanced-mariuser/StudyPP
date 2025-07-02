#include "BitonicSorter.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

const std::string BitonicSortSource = R"(
__kernel void BitonicPass(__global int* array, const uint stage, const uint pass)
{
    //возвращает уникальный идентификатор рабочей единицы (work-item)  в нулевом измерении
    uint id = get_global_id(0);
    //берем двоичное представление числа 1 (то есть ...0001) и сдвигает все его биты влево на pass позиций,
    //заполняя освободившиеся справа места нулями
    //1 << 1 = ...0010 (десятичное 2)
    //1 << 2 = ...0100 (десятичное 4)
    //возведение 2 в степень
    uint pairDistance = 1 << pass;
    uint blockWidth = 2 * pairDistance;
    //смещение внутри пары
    //даёт номер пары в блоке
    uint leftId = (id % pairDistance) + (id / pairDistance) * blockWidth;
    uint rightId = leftId + pairDistance;

    uint superBlockId = leftId / (1 << stage);
    uint directionFlag = superBlockId % 2; // 0 для блоков с сортировкой вверх, 1 для блоков с сортировкой вниз

    int leftVal = array[leftId];
    int rightVal = array[rightId];

    //условие обмена: если флаг направления 0 (вверх), то leftVal должен быть <= rightVal.
    //если флаг 1 (вниз), то leftVal должен быть >= rightVal.
    //выражение (leftVal > rightVal) дает 1, если порядок нарушен для сортировки вверх, и 0, если нет.
    if ((leftVal > rightVal) == (directionFlag == 0))
    {
        array[leftId] = rightVal;
        array[rightId] = leftVal;
    }
}
)";


BitonicSorter::BitonicSorter()
{
    Initialize();
}

void BitonicSorter::Initialize()
{
    //TODO: использовать API С++
    m_gpuHelper = std::make_unique<GpuHelper>();
    m_program = m_gpuHelper->CreateProgramFromSource(BitonicSortSource);

    cl_int err;
    m_bitonicSortKernel = clCreateKernel(m_program, "BitonicPass", &err);

    m_initialized = true;
}

void BitonicSorter::Sort(std::vector<int32_t>& data)
{
    if (!m_initialized)
    {
        throw std::runtime_error("BitonicSorter not initialized");
    }

    size_t size = data.size();
    if ((size & (size - 1)) != 0 || size == 0)
    {
        throw std::runtime_error("Array size must be a power of two and non-zero");
    }

    cl_int err;
    cl_mem buffer = clCreateBuffer(m_gpuHelper->GetContext(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                   size * sizeof(int32_t), data.data(), &err);

    RunBitonicSort(buffer, size);

    err = clEnqueueReadBuffer(m_gpuHelper->GetCommandQueue(), buffer, CL_TRUE, 0,
                              size * sizeof(int32_t), data.data(), 0, nullptr, nullptr);

    clReleaseMemObject(buffer);
}

void BitonicSorter::RunBitonicSort(cl_mem buffer, size_t size)
{
    size_t globalWorkSize = size / 2;
    size_t localWorkSize = 256;

    std::cout << "Starting bitonic sort on GPU with size: " << size << std::endl;

    clSetKernelArg(m_bitonicSortKernel, 0, sizeof(cl_mem), &buffer);

    //TODO: вычислять логорифм целочисленным способом
    //TODO: за счет чего программа выполняет сортировку миллион элемента если тут 400
    //количество запускаемых потоков напрямую зависит от размера массива
    //globalWorkSize = 1048576 / 2 = 524288 потоков
    //log2
    unsigned int numStages = 0;
    size_t temp = size;
    while (temp > 1)
    {
        temp >>= 1;
        numStages++;
    }
    //stage = 1: создаем отсортированные пары (размер 2).
    //stage = 2: из пар создаем отсортированные четверки.
    //stage = num_stages: из двух половин массива создаем финальную отсортированную последовательность.
    for (unsigned int stage = 1; stage <= numStages; ++stage)
    {
        //начинается с максимального расстояния (pass = stage-1)
        //каждый следующий проход уменьшает расстояние вдвое (pass уменьшается)
        //завершается при pass=0
        for (unsigned int pass = stage - 1; ; --pass)
        {
            clSetKernelArg(m_bitonicSortKernel, 1, sizeof(cl_uint), &stage);

            clSetKernelArg(m_bitonicSortKernel, 2, sizeof(cl_uint), &pass);

            clEnqueueNDRangeKernel(m_gpuHelper->GetCommandQueue(), m_bitonicSortKernel, 1,
                                         nullptr, &globalWorkSize, &localWorkSize, 0, nullptr, nullptr);

            //заставляет CPU ждать, пока GPU завершит текущий шаг
            clFinish(m_gpuHelper->GetCommandQueue());

            if (pass == 0) break;
        }
    }
}
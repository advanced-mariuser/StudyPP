#include "GpuHelper.h"
#include <fstream>
#include <iostream>

GpuHelper::GpuHelper()
{
    cl_int err;

    // Платформа – это реализация OpenCL от конкретного производителя
    err = clGetPlatformIDs(1, &m_platform, nullptr);

    // Запрашивает устройства определенного типа на выбранной платформе. Устройство – это конкретный вычислитель
    err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 1, &m_device, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cerr << "Failed to get GPU device, trying CPU...\n";
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 1, &m_device, nullptr);
    }

    // Контекст – это среда, в которой выполняются команды OpenCL, управляются объекты памяти, программы и ядра
    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);

    // Создает командную очередь для выбранного устройства в данном контексте. Команды (запуск ядер, передача данных) помещаются в эту очередь для выполнения устройством
    m_commandQueue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &err);
}

GpuHelper::~GpuHelper()
{
    if (m_commandQueue) clReleaseCommandQueue(m_commandQueue);
    if (m_context) clReleaseContext(m_context);
}

cl_program GpuHelper::CreateProgramFromSource(const std::string& source)
{
    const char* src = source.c_str();
    cl_int err;
    cl_program program = clCreateProgramWithSource(m_context, 1, &src, nullptr, &err);

    err = clBuildProgram(program, 1, &m_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        size_t logSize;
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "Build failed:\n" << log.data() << "\n";
        throw std::runtime_error("Program build failed");
    }

    return program;
}
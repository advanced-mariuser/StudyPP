#pragma once

#include <CL/cl.h>
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <vector>
#include <stdexcept>
#include <string>

class GpuHelper
{
public:
    GpuHelper();

    ~GpuHelper();

    cl_context GetContext() const { return m_context; }

    cl_device_id GetDevice() const { return m_device; }

    cl_command_queue GetCommandQueue() const { return m_commandQueue; }

    cl_program CreateProgramFromSource(const std::string& source);

private:
    cl_platform_id m_platform;
    cl_device_id m_device;
    cl_context m_context;
    cl_command_queue m_commandQueue;
};
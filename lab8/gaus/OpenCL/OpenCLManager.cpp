#include "OpenCLManager.h"
#include <fstream>
#include <iostream>

OpenCLManager::OpenCLManager()
        : m_isInitialized(false)
{
}

OpenCLManager::~OpenCLManager()
{
}

bool OpenCLManager::Initialize()
{
    if (m_isInitialized)
        return true;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty())
    {
        std::cerr << "OpenCLManager Error: No OpenCL platforms found." << std::endl;
        return false;
    }
    m_platform = platforms[0];

    std::vector<cl::Device> devices;
    m_platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (devices.empty())
    {
        m_platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
        if (devices.empty())
        {
            std::cerr << "OpenCLManager Error: No CPU or GPU devices found." << std::endl;
            return false;
        }
    }
    m_device = devices[0];
    std::cout << "OpenCLManager: Using device: " << m_device.getInfo<CL_DEVICE_NAME>() << std::endl;

    m_context = cl::Context(m_device);
    m_queue = cl::CommandQueue(m_context, m_device);

    m_isInitialized = true;

    return true;
}

bool OpenCLManager::LoadProgramFromFile(const std::string& filePath, const std::string& buildOptions)
{
    if (!m_isInitialized)
    {
        std::cerr << "OpenCLManager Error: Not initialized. Call Initialize() first." << std::endl;
        return false;
    }

    std::ifstream kernelFile(filePath);
    if (!kernelFile.is_open())
    {
        std::cerr << "OpenCLManager Error: Failed to open kernel file: "
                  << (std::filesystem::current_path() / filePath).string() << std::endl;
        return false;
    }

    std::string kernelCode((std::istreambuf_iterator<char>(kernelFile)),
                           std::istreambuf_iterator<char>());
    kernelFile.close();

    if (kernelCode.empty())
    {
        std::cerr << "OpenCLManager Error: Kernel file '" << filePath << "' is empty." << std::endl;
        return false;
    }

    cl::Program::Sources sources;
    sources.push_back({kernelCode.c_str(), kernelCode.length()});

    m_program = cl::Program(m_context, sources);

    std::string options = buildOptions;
    m_program.build({m_device}, options.c_str());

    std::string buildStatusLog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device);
    if (!buildStatusLog.empty() && buildStatusLog.find_first_not_of(" \t\n\r\f\v") != std::string::npos)
    {
        std::cout << "OpenCLManager Program Build Status/Log:" << std::endl;
        std::cout << buildStatusLog << std::endl;
    }

    return true;
}

cl::Context OpenCLManager::GetContext() const { return m_context; }

cl::Device OpenCLManager::GetDevice() const { return m_device; }

cl::CommandQueue OpenCLManager::GetQueue() const { return m_queue; }

cl::Program OpenCLManager::GetProgram() const { return m_program; }
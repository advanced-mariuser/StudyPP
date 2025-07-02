#pragma once
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <string>
#include <vector>
#include <filesystem> // Для работы с путями

class OpenCLManager {
public:
	OpenCLManager();
	~OpenCLManager(); // Для возможного освобождения ресурсов, хотя cl.hpp делает это автоматически

	bool Initialize();
	bool LoadProgramFromFile(const std::string& filePath, const std::string& buildOptions = "");

	cl::Context GetContext() const;
	cl::Device GetDevice() const;
	cl::CommandQueue GetQueue() const;
	cl::Program GetProgram() const; // Может понадобиться для создания разных ядер

private:
	cl::Platform m_platform;
	cl::Device m_device;
	cl::Context m_context;
	cl::CommandQueue m_queue;
	cl::Program m_program;

	bool m_isInitialized;
};


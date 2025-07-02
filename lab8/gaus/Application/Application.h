#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

#include "../BlurProcessor/ImageProcessor.h"
#include "../OpenCL/OpenCLManager.h"

class Application {
public:
	Application();
	int Run(int argc, char** argv);

private:
	enum class ActiveFilterType {
		NONE,
		GAUSSIAN_BLUR,
		MOTION_BLUR
	};

	bool Initialize(int argc, char** argv);
	void MainLoop();
	static void Cleanup();
	void SwitchFilter(ActiveFilterType filterType);

	std::string m_imagePath;
	cv::Mat m_inputImageBGRA;
	cv::Mat m_outputImage;

	OpenCLManager m_oclManager;
	std::vector<std::unique_ptr<ImageProcessor>> m_allProcessors;
	ImageProcessor* m_currentProcessor;

	ActiveFilterType m_activeFilterType;

	const std::string m_windowName = "Image Processing OpenCL";
	const std::string m_kernelFilePath = "image_filters.cl";
};

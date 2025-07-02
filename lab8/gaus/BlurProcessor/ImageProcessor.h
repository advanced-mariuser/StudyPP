#pragma once
#include "../OpenCL/OpenCLManager.h"

#include <opencv2/opencv.hpp>
#include <string>

class ImageProcessor {
public:
	virtual ~ImageProcessor() = default;

	virtual bool Initialize(OpenCLManager& oclManager) = 0;

	virtual bool ProcessImage(const cv::Mat& inputImage, cv::Mat& outputImage) = 0;

	virtual void CreateTrackbars(const std::string& windowName) = 0;
	virtual void UpdateParametersFromTrackbars(const std::string& windowName) = 0;
	virtual void OnParametersChanged(const std::string& windowName) = 0;

	virtual std::string GetName() const = 0;
};

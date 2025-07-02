#pragma once
#include "../OpenCL/OpenCLManager.h"
#include "ImageProcessor.h"

class MotionBlurProcessor : public ImageProcessor
{
public:
	MotionBlurProcessor();

	bool Initialize(OpenCLManager& oclManager) override;
	bool ProcessImage(const cv::Mat& inputImage, cv::Mat& outputImage) override;

	void CreateTrackbars(const std::string& windowName) override;
	void OnParametersChanged(const std::string& windowName) override;
	void UpdateParametersFromTrackbars(const std::string& windowName) override;

	std::string GetName() const override;

	static void StaticOnTrackbar(int pos, void* userdata);

private:
	OpenCLManager* m_oclManagerPtr;
	cl::Kernel m_kernelMotionBlurH;

	int m_sliderLength;
	const int m_maxLengthSlider = 50;

	bool m_parametersChanged;
};

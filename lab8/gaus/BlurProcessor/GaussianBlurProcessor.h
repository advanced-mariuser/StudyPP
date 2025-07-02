#pragma once
#include "../OpenCL/OpenCLManager.h"
#include "ImageProcessor.h"

class GaussianBlurProcessor final : public ImageProcessor
{
public:
	GaussianBlurProcessor();

	bool Initialize(OpenCLManager& oclManager) override;
	bool ProcessImage(const cv::Mat& inputImage, cv::Mat& outputImage) override;

	void CreateTrackbars(const std::string& windowName) override;
	void UpdateParametersFromTrackbars(const std::string& windowName) override;
	void OnParametersChanged(const std::string& windowName) override;

	std::string GetName() const override;

	static void StaticOnTrackbar(int pos, void* userdata);

private:
	static std::vector<float> GenerateGaussianKernel(int kernelRadius, float& sigmaOut);

	OpenCLManager* m_oclManagerPtr;
	cl::Kernel m_kernelH;
	cl::Kernel m_kernelTranspose;

	cl::Buffer m_clKernelWeightsBuffer;

	int m_sliderRadius;
	const int m_maxRadiusSlider = 30;

	bool m_parametersChanged;
};

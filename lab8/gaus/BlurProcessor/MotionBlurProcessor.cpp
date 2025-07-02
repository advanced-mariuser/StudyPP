#define CL_HPP_ENABLE_EXCEPTIONS

#include "MotionBlurProcessor.h"
#include <iostream>

MotionBlurProcessor::MotionBlurProcessor()
        : m_oclManagerPtr(nullptr), m_sliderLength(0), m_parametersChanged(true)
{
}

bool MotionBlurProcessor::Initialize(OpenCLManager& oclManager)
{
    m_oclManagerPtr = &oclManager;

    cl::Program program = m_oclManagerPtr->GetProgram();
    m_kernelMotionBlurH = cl::Kernel(program, "motion_blur_horizontal_rgba");

    return true;
}

std::string MotionBlurProcessor::GetName() const
{
    return "Motion Blur";
}

void MotionBlurProcessor::CreateTrackbars(const std::string& windowName)
{
    cv::createTrackbar("Length", windowName, nullptr, m_maxLengthSlider, StaticOnTrackbar, this);
    cv::setTrackbarPos("Length", windowName, m_sliderLength);
}

void MotionBlurProcessor::StaticOnTrackbar(int /*pos*/, void* userdata)
{
    if (const auto processor = static_cast<MotionBlurProcessor*>(userdata))
    {
        processor->m_parametersChanged = true;
    }
}

void MotionBlurProcessor::OnParametersChanged(const std::string& windowName)
{
    int newLength = cv::getTrackbarPos("Length", windowName);
    if (newLength == -1)
    {
        newLength = m_sliderLength;
    }

    if (newLength != m_sliderLength)
    {
        m_sliderLength = newLength;
        std::cout << "MotionBlurProcessor: Updating length = " << m_sliderLength << std::endl;
    }

    m_parametersChanged = false;
}

bool MotionBlurProcessor::ProcessImage(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (!m_oclManagerPtr || inputImage.empty() || m_kernelMotionBlurH() == nullptr)
    {
        std::cerr << "MotionBlurProcessor Error: Not initialized, input image empty, or kernel not created."
                  << std::endl;
        return false;
    }

    const cl::Context context = m_oclManagerPtr->GetContext();
    const cl::CommandQueue queue = m_oclManagerPtr->GetQueue();

    const auto clInputBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          inputImage.total() * inputImage.elemSize(), inputImage.data);
    const auto clOutputBufferMB = cl::Buffer(context, CL_MEM_WRITE_ONLY,
                                             inputImage.total() * inputImage.elemSize());

    m_kernelMotionBlurH.setArg(0, clInputBuffer);
    m_kernelMotionBlurH.setArg(1, clOutputBufferMB);
    m_kernelMotionBlurH.setArg(2, inputImage.cols);
    m_kernelMotionBlurH.setArg(3, inputImage.rows);
    m_kernelMotionBlurH.setArg(4, std::max(1, m_sliderLength));

    const cl::NDRange globalSize(inputImage.cols, inputImage.rows);
    queue.enqueueNDRangeKernel(m_kernelMotionBlurH, cl::NullRange, globalSize, cl::NullRange);

    if (outputImage.size() != inputImage.size() || outputImage.type() != inputImage.type())
    {
        outputImage.create(inputImage.size(), inputImage.type());
    }
    queue.enqueueReadBuffer(clOutputBufferMB, CL_TRUE, 0,
                            outputImage.total() * outputImage.elemSize(), outputImage.data);
    return true;
}

void MotionBlurProcessor::UpdateParametersFromTrackbars(const std::string& windowName)
{
    int newLength = cv::getTrackbarPos("Length", windowName);
    if (newLength != -1 && newLength != m_sliderLength)
    {
        m_sliderLength = newLength;
        m_parametersChanged = true;
        std::cout << "MotionBlurProcessor: Length updated to " << m_sliderLength
                  << " via UpdateParametersFromTrackbars." << std::endl;
    }
}
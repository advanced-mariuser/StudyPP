#define CL_HPP_ENABLE_EXCEPTIONS

#include "GaussianBlurProcessor.h"
#include <algorithm>
#include <cmath>
#include <iostream>

GaussianBlurProcessor::GaussianBlurProcessor()
        : m_oclManagerPtr(nullptr), m_sliderRadius(0), m_parametersChanged(true)
{
}

bool GaussianBlurProcessor::Initialize(OpenCLManager& oclManager)
{
    m_oclManagerPtr = &oclManager;
    const cl::Program program = m_oclManagerPtr->GetProgram();
    m_kernelH = cl::Kernel(program, "gaussian_blur_horizontal_rgba");
    m_kernelTranspose = cl::Kernel(program, "transpose_rgba");
    return true;
}

std::string GaussianBlurProcessor::GetName() const
{
    return "Gaussian Blur";
}

void GaussianBlurProcessor::CreateTrackbars(const std::string& windowName)
{
    cv::createTrackbar("Radius", windowName, nullptr, m_maxRadiusSlider, StaticOnTrackbar, this);
    cv::setTrackbarPos("Radius", windowName, m_sliderRadius);
}

void GaussianBlurProcessor::StaticOnTrackbar(int /*pos*/, void* userdata)
{
    if (const auto processor = static_cast<GaussianBlurProcessor*>(userdata))
    {
        processor->m_parametersChanged = true;
    }
}

void GaussianBlurProcessor::UpdateParametersFromTrackbars(const std::string& windowName)
{
    int newRadius = cv::getTrackbarPos("Radius", windowName);
    if (newRadius != -1 && newRadius != m_sliderRadius)
    {
        m_sliderRadius = newRadius;
        m_parametersChanged = true;
    }
}

void GaussianBlurProcessor::OnParametersChanged(const std::string& windowName)
{
    int newRadius = cv::getTrackbarPos("Radius", windowName);
    if (newRadius == -1)
    {
        newRadius = m_sliderRadius;
    }

    if (newRadius != m_sliderRadius || m_clKernelWeightsBuffer() == nullptr)
    {
        m_sliderRadius = newRadius;
        float sigmaCalculated;
        std::vector<float> gaussianWeights = GenerateGaussianKernel(m_sliderRadius, sigmaCalculated);
        const int kernelSize = (m_sliderRadius == 0) ? 1 : (2 * m_sliderRadius + 1);

        if (gaussianWeights.size() != kernelSize || gaussianWeights.empty())
        {
            std::cerr << "GaussianBlurProcessor Error: generating Gaussian kernel or size mismatch." << std::endl;
            return;
        }
        std::cout << "GaussianBlurProcessor: Updating kernel for radius = " << m_sliderRadius << std::endl;

        if (!m_oclManagerPtr)
        {
            return;
        }
        m_clKernelWeightsBuffer = cl::Buffer(m_oclManagerPtr->GetContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             sizeof(float) * kernelSize, gaussianWeights.data());
    }
    m_parametersChanged = false;
}

bool GaussianBlurProcessor::ProcessImage(const cv::Mat& inputImage, cv::Mat& outputImage)
{
    if (!m_oclManagerPtr || inputImage.empty())
    {
        std::cerr << "GaussianBlurProcessor Error: Not initialized or input image empty." << std::endl;
        return false;
    }
    if (m_clKernelWeightsBuffer() == nullptr)
    {
        std::cerr << "GaussianBlurProcessor Error: Kernel weights buffer not initialized." << std::endl;
        return false;
    }

    const cl::Context context = m_oclManagerPtr->GetContext();
    const cl::CommandQueue queue = m_oclManagerPtr->GetQueue();
    const int width = inputImage.cols;
    const int height = inputImage.rows;

    //Входной буфер
    const auto clInputBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          inputImage.total() * inputImage.elemSize(), (void*) inputImage.data);

    //Временный буфер (такого же размера, как входной)
    const auto clTempBuffer1 = cl::Buffer(context, CL_MEM_READ_WRITE,
                                          inputImage.total() * inputImage.elemSize());

    //Буфер для транспонированного изображения (размеры поменялись местами!)
    const auto clTransposedBuffer = cl::Buffer(context, CL_MEM_READ_WRITE,
                                               inputImage.total() * inputImage.elemSize());

    // Еще один временный буфер
    const auto clTempBuffer2 = cl::Buffer(context, CL_MEM_READ_WRITE,
                                          inputImage.total() * inputImage.elemSize());

    //Выходной буфер
    const auto clOutputBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY,
                                           inputImage.total() * inputImage.elemSize());

    //Этап 1: Первое горизонтальное размытие
    m_kernelH.setArg(0, clInputBuffer);
    m_kernelH.setArg(1, clTempBuffer1); // Результат в temp1
    m_kernelH.setArg(2, m_clKernelWeightsBuffer);
    m_kernelH.setArg(3, width);
    m_kernelH.setArg(4, height);
    m_kernelH.setArg(5, m_sliderRadius);
    queue.enqueueNDRangeKernel(m_kernelH, cl::NullRange, cl::NDRange(width, height), cl::NullRange);

    //Этап 2: Транспонирование результата первого размытия
    m_kernelTranspose.setArg(0, clTempBuffer1);
    m_kernelTranspose.setArg(1, clTransposedBuffer); // Результат в transposed
    m_kernelTranspose.setArg(2, width);
    m_kernelTranspose.setArg(3, height);
    queue.enqueueNDRangeKernel(m_kernelTranspose, cl::NullRange, cl::NDRange(width, height), cl::NullRange);

    //Этап 3: Второе горизонтальное размытие (по транспонированному изображению)
    m_kernelH.setArg(0, clTransposedBuffer);
    m_kernelH.setArg(1, clTempBuffer2); // Результат в temp2
    m_kernelH.setArg(2, m_clKernelWeightsBuffer);
    m_kernelH.setArg(3, height); // Новая ширина = старая высота
    m_kernelH.setArg(4, width);  // Новая высота = старая ширина
    m_kernelH.setArg(5, m_sliderRadius);
    queue.enqueueNDRangeKernel(m_kernelH, cl::NullRange, cl::NDRange(height, width), cl::NullRange);

    //Этап 4: Финальное транспонирование, чтобы вернуть изображение в исходную ориентацию
    m_kernelTranspose.setArg(0, clTempBuffer2);
    m_kernelTranspose.setArg(1, clOutputBuffer); // Результат в финальный буфер
    m_kernelTranspose.setArg(2, height); // Ширина = старая высота
    m_kernelTranspose.setArg(3, width);  // Высота = старая ширина
    queue.enqueueNDRangeKernel(m_kernelTranspose, cl::NullRange, cl::NDRange(height, width), cl::NullRange);

    if (outputImage.size() != inputImage.size() || outputImage.type() != inputImage.type())
    {
        outputImage.create(inputImage.size(), inputImage.type());
    }
    queue.enqueueReadBuffer(clOutputBuffer, CL_TRUE, 0,
                            outputImage.total() * outputImage.elemSize(), outputImage.data);
    return true;
}

std::vector<float> GaussianBlurProcessor::GenerateGaussianKernel(const int kernelRadius, float& sigmaOut)
{
    if (kernelRadius == 0)
    {
        sigmaOut = 0.1f;
        return {1.0f};
    }
    sigmaOut = std::max(0.1f, static_cast<float>(kernelRadius) / 3.0f);
    const int kernelSize = 2 * kernelRadius + 1;
    std::vector<float> kernelWeights(kernelSize);
    float sum = 0.0f;

    const double twoSigmaSq = 2.0 * static_cast<double>(sigmaOut) * static_cast<double>(sigmaOut);

    for (int i = 0; i < kernelSize; ++i)
    {
        const int x = i - kernelRadius;
        kernelWeights[i] = static_cast<float>(std::exp(-static_cast<double>(x * x) / twoSigmaSq));
        sum += kernelWeights[i];
    }

    if (sum != 0.0f)
    {
        for (int i = 0; i < kernelSize; ++i)
        {
            kernelWeights[i] /= sum;
        }
    } else if (kernelSize > 0)
    {
        std::ranges::fill(kernelWeights, 0.0f);
        if (kernelRadius < kernelWeights.size())
            kernelWeights[kernelRadius] = 1.0f;
    }
    return kernelWeights;
}
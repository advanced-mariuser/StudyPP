#include "GaussianBlur.h"

#include <sstream>
#include <utility>
#include <thread>
#include <mutex>
#include <cstdint>
#include <array>
#include <cmath>
#include <iostream>

GaussianBlur::GaussianBlur(std::string inputFilename, std::string outputFilename, int radius, int threadNumber)
        : m_inputFilename(std::move(inputFilename)),
          m_outputFilename(std::move(outputFilename)),
          m_radius(radius),
          m_threadNumber(threadNumber),
          m_width(0),
          m_height(0)
{
}

void GaussianBlur::ApplyGaussianBlur()
{
    if (m_radius < 0)
    {
        std::cerr << "Invalid radius" << std::endl;
        return;
    }

    sf::Image image;
    if (!image.loadFromFile(m_inputFilename))
    {
        std::cerr << "Failed to load image!" << std::endl;
        return;
    }

    if (m_radius == 0)
    {
        if (!image.saveToFile(m_outputFilename))
        {
            std::cerr << "Failed to save image!" << std::endl;
        }
    } else
    {
        m_width = image.getSize().x;
        m_height = image.getSize().y;

        const auto* pixelData = reinterpret_cast<const uint8_t*>(image.getPixelsPtr());
        std::vector<uint8_t> pixels(pixelData, pixelData + (m_width * m_height * 4));

        ApplyGaussianBlurForPixels(pixels);


        sf::Image blurredImage;
        blurredImage.create(m_width, m_height, pixels.data());

        if (!blurredImage.saveToFile(m_outputFilename))
        {
            std::cerr << "Failed to save image!" << std::endl;
        }
    }
}

void GaussianBlur::ApplyGaussianBlurForPixels(std::vector<uint8_t> &pixels)
{
    if (m_radius <= 0 || m_width == 0 || m_height == 0)
    {
        std::cerr << "Invalid radius or image size" << std::endl;
        return;
    }

    ApplyGaussianBlurYOnly(pixels);

    ApplyGammaCorrection(pixels, 1.5);
}

std::vector<float> GaussianBlur::CreateGaussianKernel(int radius)
{
    std::vector<float> kernel(2 * radius + 1);
    float sigma = radius / 2.0f;
    float sum = 0.0f;

    for (int i = -radius; i <= radius; ++i)
    {
        kernel[i + radius] = std::exp(-(i * i) / (2 * sigma * sigma));
        sum += kernel[i + radius];
    }

    for (auto &value: kernel)
    {
        value /= sum;
    }

    return kernel;
}

void GaussianBlur::SetRadius(int radius)
{
    m_radius = radius;
}

int GaussianBlur::GetRadius() const
{
    return m_radius;
}

void GaussianBlur::SetImageSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

int GaussianBlur::GetWidth() const
{
    return m_width;
}

int GaussianBlur::GetHeight() const
{
    return m_height;
}

void GaussianBlur::ApplyGaussianBlurYOnly(std::vector<uint8_t> &pixels)
{
    if (m_radius <= 0 || m_width == 0 || m_height == 0)
    {
        std::cerr << "Invalid radius or image size" << std::endl;
        return;
    }

    std::vector<float> kernel = CreateGaussianKernel(m_radius);

    std::vector<uint8_t> blurredPixels(pixels.size());
    auto processRows = [&](
            int startY,
            int endY,
            const std::vector<uint8_t> &startPixels,
            std::vector<uint8_t> &blurPixels
    )
    {
        for (int y = startY; y < endY; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                ApplyKernelToPixelY(x, y, startPixels, kernel, blurPixels);
            }
        }
    };

    std::vector<std::jthread> threads;
    int rowsPerThread = m_height / m_threadNumber;
    int remainingRows = m_height % m_threadNumber;

    int startY = 0;
    for (int i = 0; i < m_threadNumber; ++i)
    {
        int endY = startY + rowsPerThread + (i < remainingRows ? 1 : 0);
        threads.emplace_back(processRows, startY, endY, std::ref(pixels), std::ref(blurredPixels));
        startY = endY;
    }

    for (auto &thread: threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    std::vector<uint8_t> transposedPixels = TransposeImage(blurredPixels);
    std::swap(m_width, m_height);

    std::vector<uint8_t> finalPixels(transposedPixels.size());
    threads.clear();
    startY = 0;
    int rowsPerThread2 = m_height / m_threadNumber;
    int remainingRows2 = m_height % m_threadNumber;
    for (int i = 0; i < m_threadNumber; ++i)
    {
        int endY = startY + rowsPerThread2 + (i < remainingRows2 ? 1 : 0);
        threads.emplace_back(processRows, startY, endY, std::ref(transposedPixels), std::ref(finalPixels));
        startY = endY;
    }

    for (auto &thread: threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    std::vector<uint8_t> resultPixels = TransposeImage(finalPixels);
    std::swap(m_width, m_height);

    pixels = std::move(resultPixels);
}

std::vector<uint8_t> GaussianBlur::TransposeImage(const std::vector<uint8_t> &pixels) const
{
    std::vector<uint8_t> transposedPixels(pixels.size());

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int originalIndex = (y * m_width + x) * 4;
            int transposedIndex = (x * m_height + y) * 4;

            transposedPixels[transposedIndex] = pixels[originalIndex];
            transposedPixels[transposedIndex + 1] = pixels[originalIndex + 1];
            transposedPixels[transposedIndex + 2] = pixels[originalIndex + 2];
            transposedPixels[transposedIndex + 3] = pixels[originalIndex + 3];
        }
    }

    return transposedPixels;
}

void
GaussianBlur::ApplyKernelToPixelY(int x, int y, const std::vector<uint8_t> &pixels, const std::vector<float> &kernel,
                                  std::vector<uint8_t> &blurredPixels) const
{
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
    int kernelRadius = kernel.size() / 2;

    for (int ky = -kernelRadius; ky <= kernelRadius; ++ky)
    {
        int ny = y + ky;

        if (ny >= 0 && ny < m_height)
        {
            int pixelIndex = (ny * m_width + x) * 4;
            float weight = kernel[ky + kernelRadius];

            r += pixels[pixelIndex] * weight;
            g += pixels[pixelIndex + 1] * weight;
            b += pixels[pixelIndex + 2] * weight;
            a += pixels[pixelIndex + 3] * weight;
        }
    }

    int pixelIndex = (y * m_width + x) * 4;
    blurredPixels[pixelIndex] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, r)));
    blurredPixels[pixelIndex + 1] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, g)));
    blurredPixels[pixelIndex + 2] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, b)));
    blurredPixels[pixelIndex + 3] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, a)));
}

void GaussianBlur::ApplyGammaCorrection(std::vector<uint8_t> &pixels, float gamma)
{
    static std::array<uint8_t, 256> gammaTable;
    static float lastGamma = 0.0f;

    if (lastGamma != gamma)
    {
        lastGamma = gamma;
        for (int i = 0; i < 256; ++i)
        {
            gammaTable[i] = static_cast<uint8_t>(std::pow(i / 255.0f, gamma) * 255.0f);
        }
    }

    for (size_t i = 0; i < pixels.size(); i += 4)
    {
        pixels[i] = gammaTable[pixels[i]];
        pixels[i + 1] = gammaTable[pixels[i + 1]];
        pixels[i + 2] = gammaTable[pixels[i + 2]];
    }
}

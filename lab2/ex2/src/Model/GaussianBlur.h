#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <cstdint>
#include <SFML/Graphics.hpp>

class GaussianBlur
{
public:
    GaussianBlur(std::string inputFilename, std::string outputFilename, int radius, int threadNumber);

    void ApplyGaussianBlur();
    void ApplyGaussianBlurForPixels(std::vector<uint8_t>& pixels);

    // Геттеры для размеров изображения
    int GetWidth() const;
    int GetHeight() const;


    void SetImageSize(int width, int height);

    void SetRadius(int radius);

private:
    std::string m_inputFilename;
    std::string m_outputFilename;
    int m_radius;
    int m_threadNumber;
    int m_width;
    int m_height;

    void ApplyGaussianBlurYOnly(std::vector<uint8_t>& pixels);
    std::vector<uint8_t> TransposeImage(const std::vector<uint8_t>& pixels) const;

    void ApplyGammaCorrection(std::vector<uint8_t>& pixels, float gamma);
    std::vector<float> CreateGaussianKernel(int radius);

    void ApplyKernelToPixelY(int x, int y, const std::vector<uint8_t>& pixels,
                             const std::vector<float>& kernel, std::vector<uint8_t>& blurredPixels) const;

    void RunParallelProcessing(int start, int end,
                               const std::function<void(int, int, const std::vector<uint8_t>&, std::vector<uint8_t>&)>& process,
                               const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

    int GetRadius() const;
};

#endif // GAUSSIAN_BLUR_H

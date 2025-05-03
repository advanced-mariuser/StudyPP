#ifndef GAUSSIANBLURCONTROLLER_H
#define GAUSSIANBLURCONTROLLER_H

#include <SFML/Graphics.hpp>
#include "../Model/GaussianBlur.h"
#include "../View/GaussianBlurView.h"

class GaussianBlurController
{
public:
    GaussianBlurController(GaussianBlur &model, GaussianBlurView &view);

    void HandleEvent(const sf::Event &event);

    void Run();

private:
    GaussianBlur &m_model;
    GaussianBlurView &m_view;

    void ApplyGaussianBlurToImage(const std::string &imagePath, int radius);

    void UpdateViewWithBlurredImage(const std::vector<uint8_t> &pixels, sf::Image &image);

    std::vector<uint8_t> ApplyBlurToPixels(std::vector<uint8_t> pixels);

    std::vector<uint8_t> PrepareModelForBlur(int radius, sf::Image &image);

    static void LoadImage(const std::string &imagePath, sf::Image &image);
};

#endif // GAUSSIANBLURCONTROLLER_H

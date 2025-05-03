#include "GaussianBlurController.h"
#include <cstdint>

GaussianBlurController::GaussianBlurController(GaussianBlur &model, GaussianBlurView &view)
        : m_model(model), m_view(view)
{
}

void GaussianBlurController::Run()
{
    while (m_view.GetWindow().isOpen())
    {
        sf::Event event {};
        while (m_view.GetWindow().pollEvent(event))
        {
            HandleEvent(event);
        }

        m_view.Draw();
    }
}

void GaussianBlurController::HandleEvent(const sf::Event &event)
{
    if (event.type == sf::Event::Closed)
    {
        m_view.GetWindow().close();
    }

    m_view.GetSlider().HandleEvent(event, m_view.GetWindow());

    m_view.GetImageSelector().HandleEvent(event, m_view.GetWindow());

    std::string selectedImagePath = m_view.GetImageSelector().GetSelectedImage();
    if (!selectedImagePath.empty())
    {
        ApplyGaussianBlurToImage(selectedImagePath, m_view.GetSlider().GetValue());
    }
}

void GaussianBlurController::ApplyGaussianBlurToImage(const std::string &imagePath, int radius)
{
    sf::Image image;
    LoadImage(imagePath, image);

    if (m_view.GetSlider().GetValue() != 0)
    {
        std::vector<uint8_t> pixels = PrepareModelForBlur(radius, image);
        pixels = ApplyBlurToPixels(pixels);
        UpdateViewWithBlurredImage(pixels, image);
    } else
    {
        sf::Texture texture;
        texture.loadFromImage(image);
        m_view.GetImageSelector().SetTexture(texture);
    }
}

void GaussianBlurController::LoadImage(const std::string &imagePath, sf::Image &image)
{
    if (!image.loadFromFile(imagePath))
    {
        std::cerr << "Failed to load image!" << std::endl;
        throw std::runtime_error("Failed to load image");
    }
}

std::vector<uint8_t> GaussianBlurController::PrepareModelForBlur(int radius, sf::Image &image)
{
    int width = image.getSize().x;
    int height = image.getSize().y;

    m_model.SetImageSize(width, height);
    m_model.SetRadius(radius);

    const auto* pixelData = reinterpret_cast<const uint8_t*>(image.getPixelsPtr());
    std::vector<uint8_t> pixels(pixelData, pixelData + (width * height * 4));

    return pixels;
}

std::vector<uint8_t> GaussianBlurController::ApplyBlurToPixels(std::vector<uint8_t> pixels)
{
    m_model.ApplyGaussianBlurForPixels(pixels);
    return pixels;
}

void GaussianBlurController::UpdateViewWithBlurredImage(const std::vector<uint8_t> &pixels, sf::Image &image)
{
    int width = image.getSize().x;
    int height = image.getSize().y;

    sf::Image blurredImage;
    blurredImage.create(width, height, pixels.data());

    sf::Texture texture;
    texture.loadFromImage(blurredImage);

    m_view.GetImageSelector().SetTexture(texture);
}
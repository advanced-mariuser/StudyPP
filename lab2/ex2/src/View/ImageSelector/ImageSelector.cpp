#include "ImageSelector.h"
#include <iostream>
#include <windows.h>

ImageSelector::ImageSelector(float x, float y, float width, float height)
        : m_rectangle(sf::Vector2f(width, height)), m_width(width), m_height(height)
{
    m_rectangle.setPosition(x, y);
    m_rectangle.setFillColor(sf::Color::Blue);
    m_rectangle.setOutlineThickness(2);
    m_rectangle.setOutlineColor(sf::Color::White);
}

void ImageSelector::Draw(sf::RenderWindow& window)
{
    if (!m_selectedImage.empty())
    {
        if (!m_texture.getSize().x || !m_texture.getSize().y) {
            if (!m_texture.loadFromFile(m_selectedImage))
            {
                std::cout << "Failed to load image!" << std::endl;
                return;
            }
        }

        sf::Sprite sprite(m_texture);

        float scaleX = m_width / static_cast<float>(m_texture.getSize().x);
        float scaleY = m_height / static_cast<float>(m_texture.getSize().y);
        float scale = std::min(scaleX, scaleY);

        sprite.setScale(scale, scale);

        float spriteWidth = m_texture.getSize().x * scale;
        float spriteHeight = m_texture.getSize().y * scale;

        float posX = m_rectangle.getPosition().x + (m_width - spriteWidth) / 2;
        float posY = m_rectangle.getPosition().y + (m_height - spriteHeight) / 2;

        sprite.setPosition(posX, posY);

        window.draw(sprite);
    }
    else
    {
        window.draw(m_rectangle);
    }
}

void ImageSelector::HandleEvent(const sf::Event& event, sf::RenderWindow& window)
{
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        if (m_rectangle.getGlobalBounds().contains(mousePos))
        {
            std::string selectedImagePath = OpenFileDialog();
            if (!selectedImagePath.empty())
            {
                m_selectedImage = selectedImagePath;
            }
        }
    }
}

std::string ImageSelector::OpenFileDialog()
{
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "";

    if (GetOpenFileName(&ofn))
    {
        return std::string(fileName);
    }
    return "";
}

std::string ImageSelector::GetSelectedImage() const
{
    return m_selectedImage;
}

void ImageSelector::SetTexture(const sf::Texture& texture)
{
    m_texture = texture;
}
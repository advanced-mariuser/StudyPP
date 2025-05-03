#include <cmath>
#include "Slider.h"

Slider::Slider(float x, float y, float width, int minValue, int maxValue)
        : m_position(x, y), m_size(width, 10.0f), m_minValue(minValue), m_maxValue(maxValue), m_value(minValue)
{
    // Инициализация прямоугольника слайдера
    m_sliderRect.setSize(sf::Vector2f(m_size.x, m_size.y));
    m_sliderRect.setFillColor(sf::Color(100, 100, 100));
    m_sliderRect.setPosition(m_position);

    // Инициализация ползунка
    m_handle.setRadius(10.0f);
    m_handle.setFillColor(sf::Color::White);
    m_handle.setPosition(m_position.x, m_position.y - 5.0f);

    // Инициализация текста
    std::string fontPath = std::string(PROJECT_ROOT) + "/assets/font/arialmt.ttf";
    if (!m_font.loadFromFile(fontPath))
    {
        std::cerr << "Failed to load font from: " << fontPath << std::endl;
    }
    m_text.setFont(m_font);
    m_text.setCharacterSize(20);
    m_text.setFillColor(sf::Color::White);
    UpdateText();
}

void Slider::Draw(sf::RenderWindow &window)
{
    window.draw(m_sliderRect);
    window.draw(m_handle);
    window.draw(m_text);
}

void Slider::HandleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        if (m_handle.getGlobalBounds().contains(mousePos))
        {
            m_isDragging = true;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        m_isDragging = false;
    }
    else if (event.type == sf::Event::MouseMoved && m_isDragging)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float newX = std::max(m_position.x, std::min(m_position.x + m_size.x, mousePos.x));
        m_handle.setPosition(newX, m_handle.getPosition().y);

        float normalizedValue = (newX - m_position.x) / m_size.x;
        m_value = static_cast<int>(std::round(m_minValue + normalizedValue * (m_maxValue - m_minValue)));
        UpdateText();
    }
}

int Slider::GetValue() const
{
    return m_value;
}

void Slider::UpdateText()
{
    std::stringstream ss;
    ss << m_value;
    m_text.setString(ss.str());
    m_text.setPosition(m_position.x - 60.0f, m_position.y - 5.0f);
}
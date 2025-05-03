#ifndef GAUSSIAN_BLUR_SLIDER_H
#define GAUSSIAN_BLUR_SLIDER_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>

class Slider
{
public:
    Slider(float x, float y, float width, int minValue, int maxValue);

    void Draw(sf::RenderWindow &window);

    void HandleEvent(const sf::Event &event, const sf::RenderWindow &window);

    int GetValue() const;

private:
    void UpdateText();

    sf::Vector2f m_position;
    sf::Vector2f m_size;

    int m_minValue;
    int m_maxValue;
    int m_value;

    sf::RectangleShape m_sliderRect;
    sf::CircleShape m_handle;
    bool m_isDragging = false;

    sf::Font m_font;
    sf::Text m_text;
};

#endif //GAUSSIAN_BLUR_SLIDER_H

#include "GameLifeView.h"

GameLifeView::GameLifeView(sf::RenderWindow &window, const GameLife &model)
        : m_window(window)
{
}

void GameLifeView::Draw(GameLife model)
{
    m_window.clear(sf::Color::White);

    sf::RectangleShape cell(sf::Vector2f(10.f, 10.f));
    for (size_t y = 0; y < model.getSizeY(); ++y)
    {
        for (size_t x = 0; x < model.getSizeX(); ++x)
        {
            if (model.isAlive(x, y))
            {
                cell.setPosition(static_cast<float>(x * 10), static_cast<float>(y * 10));
                cell.setFillColor(sf::Color::Black);
                m_window.draw(cell);
            }
        }
    }

    m_window.display();
}

sf::RenderWindow &GameLifeView::GetWindow()
{
    return m_window;
}
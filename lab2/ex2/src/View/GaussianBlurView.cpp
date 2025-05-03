#include "GaussianBlurView.h"

GaussianBlurView::GaussianBlurView(sf::RenderWindow &window)
        : m_window(window), m_slider(200.0f, 500.0f, 400.0f, 0, 100), m_imageSelector(200.0f, 50.0f, 400.0f, 300.0f) {}

void GaussianBlurView::Draw()
{
    m_window.clear(sf::Color::Black);

    m_slider.Draw(m_window);
    m_imageSelector.Draw(m_window);

    m_window.display();
}

Slider &GaussianBlurView::GetSlider()
{
    return m_slider;
}

sf::RenderWindow &GaussianBlurView::GetWindow()
{
    return m_window;
}

ImageSelector& GaussianBlurView::GetImageSelector()
{
    return m_imageSelector;
}
#ifndef GAUSSIANBLURVIEW_H
#define GAUSSIANBLURVIEW_H

#include <SFML/Graphics.hpp>
#include "Slider/Slider.h"
#include "ImageSelector/ImageSelector.h"

class GaussianBlurView
{
public:
    explicit GaussianBlurView(sf::RenderWindow &window);

    void Draw();

    Slider &GetSlider();

    sf::RenderWindow &GetWindow();

    ImageSelector &GetImageSelector();

private:
    sf::RenderWindow &m_window;

    Slider m_slider;
    ImageSelector m_imageSelector;
};

#endif // GAUSSIANBLURVIEW_H

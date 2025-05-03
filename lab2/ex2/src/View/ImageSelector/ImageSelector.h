#ifndef IMAGE_SELECTOR_H
#define IMAGE_SELECTOR_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

//Возможно controller не должен взаимодейтсвовать с imageselector
class ImageSelector
{
public:
    ImageSelector(float x, float y, float width, float height);

    void Draw(sf::RenderWindow &window);

    void HandleEvent(const sf::Event &event, sf::RenderWindow &window);

    std::string GetSelectedImage() const;

    void SetTexture(const sf::Texture &texture);

private:
    static std::string OpenFileDialog();

    sf::RectangleShape m_rectangle;
    sf::Texture m_texture;
    std::vector<std::string> m_imagePaths;
    std::string m_selectedImage;
    float m_width;
    float m_height;
};

#endif // IMAGE_SELECTOR_H
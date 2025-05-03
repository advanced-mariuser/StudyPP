#ifndef GAME_LIFE_GAMELIFEVIEW_H
#define GAME_LIFE_GAMELIFEVIEW_H

#include <SFML/Graphics.hpp>
#include "../Model/GameLife.h"

class GameLifeView
{
public:
    GameLifeView(sf::RenderWindow &window, const GameLife &model);

    void Draw(GameLife model);

    sf::RenderWindow &GetWindow();

private:
    sf::RenderWindow &m_window;
};

#endif //GAME_LIFE_GAMELIFEVIEW_H

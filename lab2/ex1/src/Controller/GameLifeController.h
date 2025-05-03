#ifndef GAME_LIFE_GAMELIFECONTROLLER_H
#define GAME_LIFE_GAMELIFECONTROLLER_H

#include <SFML/Graphics.hpp>
#include "../Model/GameLife.h"
#include "../View/GameLifeView.h"

class GameLifeController {
public:
    GameLifeController(GameLife &model, GameLifeView &view);
    void Run();

private:
    GameLife &m_model;
    GameLifeView &m_view;
};

#endif //GAME_LIFE_GAMELIFECONTROLLER_H

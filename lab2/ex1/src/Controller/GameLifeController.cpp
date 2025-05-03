#include <chrono>
#include "GameLifeController.h"

GameLifeController::GameLifeController(GameLife &model, GameLifeView &view)
        : m_model(model), m_view(view)
{
}

void GameLifeController::Run()
{
    while (m_view.GetWindow().isOpen())
    {
        sf::Event event{};
        while (m_view.GetWindow().pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                m_view.GetWindow().close();
        }

        auto start = std::chrono::high_resolution_clock::now();
        m_model.UpdateState();
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

        m_view.GetWindow().setTitle(std::to_string(elapsed) + "ms");

        m_view.Draw(m_model);

        sf::sleep(sf::milliseconds(50));
    }
}
#ifndef GAME_LIFE_H
#define GAME_LIFE_H

#include <iostream>
#include <vector>
#include <string>
#include <random>

class GameLife
{
public:
    GameLife(std::string outputFilename, size_t width, size_t height, float probability);

    GameLife(std::string inputFilename, int threadNumber, const std::string &outputFilename);

    void GenerateGame();

    void DoStep();

    void ReadInputFile();

    void UpdateState();

    bool isAlive(int x, int y);

    int getSizeX();

    int getSizeY();

private:
    std::vector<std::vector<char>> m_gameField;
    std::vector<std::vector<char>> m_nextField;
    std::string m_outputFilename;
    std::string m_inputFilename;
    int m_threadNumber;
    size_t m_width{};

    bool m_useGameField = true;

    size_t m_height{};

    float m_probability{};

    void GenerateOutputFile();

    bool GenerateCell();

    std::mt19937 m_generator;

    std::uniform_real_distribution<> m_distribution;

    int CountLiveNeighbors(const std::vector<std::vector<char>> &field, size_t y, size_t x) const;

    void ComputeNextState(const std::vector<std::vector<char>> &currentField, std::vector<std::vector<char>> &nextField,
                          size_t startRow, size_t endRow);
};

#endif // GAME_LIFE_H

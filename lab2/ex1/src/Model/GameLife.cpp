#include "GameLife.h"

#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>

GameLife::GameLife(std::string outputFilename, size_t width, size_t height, float probability)
        : m_outputFilename(std::move(outputFilename)),
          m_width(width),
          m_height(height),
          m_probability(probability),
          m_generator(std::random_device{}()),
          m_distribution(0.0, 1.0)
{
    if (m_probability < 0.0f || m_probability > 1.0f)
    {
        throw std::invalid_argument("Probability must be between 0 and 1");
    }

    m_gameField.resize(height, std::vector<char>(width, ' '));
}

GameLife::GameLife(std::string inputFilename, int threadNumber, const std::string &outputFilename = "")
        : m_inputFilename(std::move(inputFilename)),
          m_threadNumber(threadNumber)
{
    if (!outputFilename.empty())
    {
        m_outputFilename = outputFilename;
    } else
    {
        m_outputFilename = inputFilename;
    }
}

void GameLife::GenerateGame()
{
    for (auto &row: m_gameField)
    {
        for (char &val: row)
        {
            if (GenerateCell())
                val = '#';
        }
    }

    GenerateOutputFile();
}

void GameLife::GenerateOutputFile()
{
    std::ofstream outputFile(m_outputFilename);
    if (!outputFile)
    {
        throw std::runtime_error("Error: Unable to open file " + m_outputFilename);
    }

    outputFile << m_width << " " << m_height << std::endl;

    for (const auto &row: m_gameField)
    {
        for (char val: row)
        {
            outputFile << val;
        }
        outputFile << std::endl;
    }

    if (!outputFile.flush())
    {
        throw std::runtime_error("Error: Failed to write data to file " + m_outputFilename);
    }
}

bool GameLife::GenerateCell()
{
    return m_distribution(m_generator) < m_probability;
}

void GameLife::DoStep()
{
    ReadInputFile();
    UpdateState();
    GenerateOutputFile();
}

void GameLife::ReadInputFile()
{
    std::ifstream inputFile(m_inputFilename);
    if (!inputFile)
    {
        throw std::runtime_error("Error: Unable to open file " + m_inputFilename);
    }

    inputFile >> m_width >> m_height;
    m_gameField.resize(m_height, std::vector<char>(m_width, ' '));
    m_nextField.resize(m_height, std::vector<char>(m_width, ' '));

    for (size_t y = 0; y < m_height; ++y)
    {
        std::string line;
        std::getline(inputFile, line);

        for (size_t x = 0; x < std::min(line.size(), m_width); ++x)
        {
            m_gameField[y][x] = line[x];
        }
    }
}

void GameLife::UpdateState()
{
    auto &currentField = m_useGameField ? m_gameField : m_nextField;
    auto &nextField = m_useGameField ? m_nextField : m_gameField;

    std::vector<std::jthread> threads;
    size_t rowsPerThread = m_height / m_threadNumber;

    for (int i = 0; i < m_threadNumber; ++i)
    {
        size_t startRow = i * rowsPerThread;
        size_t endRow = (i == m_threadNumber - 1) ? m_height : startRow + rowsPerThread;

        threads.emplace_back(&GameLife::ComputeNextState, this, std::cref(currentField), std::ref(nextField), startRow, endRow);
    }

    for (auto &thread: threads)
    {
        thread.join();
    }

    m_useGameField = !m_useGameField;
}

void GameLife::ComputeNextState(const std::vector<std::vector<char>> &currentField,
                                std::vector<std::vector<char>> &nextField,
                                size_t startRow, size_t endRow)
{
    for (size_t y = startRow; y < endRow; ++y)
    {
        for (size_t x = 0; x < m_width; ++x)
        {
            int liveNeighbors = CountLiveNeighbors(currentField, y, x);
            nextField[y][x] = (currentField[y][x] == '#')
                              ? ((liveNeighbors == 2 || liveNeighbors == 3) ? '#' : ' ')
                              : ((liveNeighbors == 3) ? '#' : ' ');
        }
    }
}

int GameLife::CountLiveNeighbors(const std::vector<std::vector<char>> &field, size_t y, size_t x) const
{
    int count = 0;

    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dy == 0) continue;

            size_t ny = (y + dy + m_height) % m_height;
            size_t nx = (x + dx + m_width) % m_width;

            if (field[ny][nx] == '#')
            {
                ++count;
            }
        }
    }
    return count;
}

bool GameLife::isAlive(int x, int y)
{
    if (x >= 0 && x < static_cast<int>(m_width) && y >= 0 && y < static_cast<int>(m_height))
    {
        return m_gameField[y][x] == '#';
    }
    return false;
}

int GameLife::getSizeX()
{
    return m_gameField[0].size();
}

int GameLife::getSizeY()
{
    return m_gameField.size();
}
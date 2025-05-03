#include "src/Model/GameLife.h"
#include "src/Controller/GameLifeController.h"
#include "src/View/GameLifeView.h"

struct Args
{
    std::string mode;
    std::string inputFilename;
    std::string outputFilename;
    int numberThreads = 0;
    size_t width = 10;
    size_t height = 10;
    float probability = 1.0f;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 4)
    {
        throw std::invalid_argument(
                "Usage: game-life generate OUTPUT_FILE_NAME WIDTH HEIGHT PROBABILITY\n"
                "   Or: game-life step INPUT_FILE_NAME NUM_THREADS <OUTPUT_FILE_NAME>\n"
                "   Or: game-life visualize INPUT_FILE_NAME NUM_THREADS");
    }

    Args args;
    args.mode = argv[1];

    if (args.mode == "generate")
    {
        if (argc != 6)
            throw std::invalid_argument("Invalid arguments for \"generate\" mode.");
        args.outputFilename = argv[2];
        args.width = std::stoull(argv[3]);
        args.height = std::stoull(argv[4]);
        args.probability = std::stof(argv[5]);
    } else if (args.mode == "step" || args.mode == "visualize")
    {
        args.inputFilename = argv[2];
        args.numberThreads = std::stoi(argv[3]);
        if (argc == 4)
            args.outputFilename = argv[2];
        else
            args.outputFilename = argv[4];
    } else
    {
        throw std::invalid_argument(
                R"(Invalid mode. Use "generate" for generate game or "step" for next step of game.)");
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        auto args = ParseArgs(argc, argv);
        if (args.mode == "generate")
        {
            GameLife gameLife(args.outputFilename, args.width, args.height, args.probability);
            gameLife.GenerateGame();
        } else if (args.mode == "step")
        {
            GameLife gameLife(args.inputFilename, args.numberThreads, args.outputFilename);
            gameLife.DoStep();
        } else
        {
            GameLife model(args.inputFilename, args.numberThreads, args.outputFilename);
            model.ReadInputFile();
            sf::RenderWindow window(
                    sf::VideoMode(static_cast<unsigned int>(model.getSizeX() * 10),
                                  static_cast<unsigned int>(model.getSizeY() * 10)),
                    "Game of Life");
            GameLifeView view(window, model);
            GameLifeController controller(model, view);
            controller.Run();
        }
    } catch (const std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

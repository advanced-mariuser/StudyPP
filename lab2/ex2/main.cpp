#include "src/Model/GaussianBlur.h"
#include "src/View/GaussianBlurView.h"
#include "src/Controller/GaussianBlurController.h"

struct Args
{
    std::string inputFilename;
    std::string outputFilename;
    int radius = 0;
    int numberThreads = 0;
    bool visualize = false;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 5)
    {
        throw std::invalid_argument(
                "Usage: INPUT_FILE OUTPUT_FILE RADIUS NUM_THREADS [--visualize]\n");
    }

    Args args;

    args.inputFilename = argv[1];
    args.outputFilename = argv[2];
    args.radius = std::stoi(argv[3]);

    auto threadsNumber = std::stoi(argv[4]);
    if (threadsNumber <= 0)
    {
        throw std::invalid_argument(
                "Usage: INPUT_FILE OUTPUT_FILE RADIUS NUM_THREADS [--visualize]\n");
    }
    args.numberThreads = threadsNumber;

    for (int i = 5; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--visualize")
        {
            args.visualize = true;
        }
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        auto args = ParseArgs(argc, argv);

        GaussianBlur model(args.inputFilename, args.outputFilename, args.radius, args.numberThreads);
        if (args.visualize)
        {
            sf::RenderWindow window(sf::VideoMode(800, 600), "Gaussian Blur");
            GaussianBlurView view(window);
            GaussianBlurController controller(model, view);

            controller.Run();
        } else
        {
            model.ApplyGaussianBlur();
        }
    } catch (const std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include "src/Melody.h"
#include <string>
#include <stdexcept>
#include <iostream>

struct Args
{
    std::string FilePath;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 2)
    {
        throw std::invalid_argument(
                "Usage: " + std::string(argv[0]) + " <file_path>\n"
        );
    }

    Args args{};
    args.FilePath = argv[1];
    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        Args args = ParseArgs(argc, argv);
        Melody melody(args.FilePath);
        melody.Play();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

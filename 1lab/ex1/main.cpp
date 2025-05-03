#include "src/ArchiveCreator.h"

struct Args
{
    std::string flag;
    std::string archiveName;
    int numberProcesses = 0;
    std::vector<std::string> inputFiles;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 3)
    {
        throw std::invalid_argument(
                "Usage: make-archive -S ARCHIVE-NAME [INPUT-FILES]\n"
                "   Or: make-archive -P NUM-PROCESSES ARCHIVE-NAME [INPUT-FILES]");
    }

    Args args;
    args.flag = argv[1];

    if (args.flag == "-P")
    {
        if (argc < 4) throw std::invalid_argument("Invalid arguments for -P mode.");
        args.numberProcesses = std::stoi(argv[2]);
        args.archiveName = argv[3];
        for (int i = 4; i < argc; ++i)
        {
            args.inputFiles.push_back(argv[i]);
        }
    } else if (args.flag == "-S")
    {
        args.archiveName = argv[2];
        for (int i = 3; i < argc; ++i)
        {
            args.inputFiles.push_back(argv[i]);
        }
    } else
    {
        throw std::invalid_argument("Invalid flag. Use -S for sequential mode or -P for parallel mode.");
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        auto args = ParseArgs(argc, argv);
        ArchiveCreator archiveCreator(args.archiveName, args.inputFiles);

        if (args.flag == "-P")
        {
            archiveCreator.RunParallel(args.numberProcesses);
        } else if (args.flag == "-S")
        {
            archiveCreator.RunSequential();
        }
    } catch (const std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

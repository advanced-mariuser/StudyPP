#include "src/Extractor.h"

struct ProgramArgs
{
    bool isParallel;
    int numProcesses;
    std::string archiveName;
    std::string outputFolder;
};

bool ParseArgs(int argc, char* argv[], ProgramArgs &args)
{
    if (argc < 4)
    {
        return false;
    }

    std::string mode = argv[1];

    if (mode == "-S" && argc == 4)
    {
        args.isParallel = false;
        args.archiveName = argv[2];
        args.outputFolder = argv[3];
        return true;
    }
    if (mode == "-P" && argc == 5)
    {
        args.isParallel = true;
        args.numProcesses = std::stoi(argv[2]);
        args.archiveName = argv[3];
        args.outputFolder = argv[4];
        return true;
    }
    return false;
}

void PrintUsage()
{
    std::cerr << "Usage:\n"
              << "  extract-files -S ARCHIVE-NAME OUTPUT-FOLDER\n"
              << "  extract-files -P NUM-PROCESSES ARCHIVE-NAME OUTPUT-FOLDER\n";
}

int main(int argc, char* argv[])
{
    ProgramArgs args;

    if (!ParseArgs(argc, argv, args))
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    Extractor extractor(args.archiveName, args.outputFolder);

    if (args.isParallel)
    {
        extractor.RunParallel(args.numProcesses);
    } else
    {
        extractor.RunSequential();
    }

    return EXIT_SUCCESS;
}

#include "src/Simulation.h"
#include <iostream>

struct Args
{
    Money initialCash;
    bool parallel;
};

Args ParseArgs(int argc, char* argv[])
{
    if (argc < 3)
    {
        throw std::invalid_argument(
                "Usage: " + std::string(argv[0]) + " <initial_cash> <parallel|sequential>\n"
        );
    }

    Args args{};

    try
    {
        args.initialCash = std::stoll(argv[1]);
    } catch (const std::invalid_argument&)
    {
        throw std::invalid_argument("Initial cash must be a valid number.\n");
    } catch (const std::out_of_range&)
    {
        throw std::invalid_argument("Initial cash is out of range.\n");
    }

    std::string mode = argv[2];
    if (mode == "parallel")
    {
        args.parallel = true;
    } else if (mode == "sequential")
    {
        args.parallel = false;
    } else
    {
        throw std::invalid_argument("Mode must be 'parallel' or 'sequential'.\n");
    }

    return args;
}

int main(int argc, char* argv[])
{
    try
    {
        Args args = ParseArgs(argc, argv);

        Simulation simulation(args.parallel, args.initialCash);
        simulation.RunSimulation();

        std::cout << "Total bank operations: " << simulation.GetBank().GetOperationsCount() << std::endl;
        std::cout << "Remaining cash: " << simulation.GetBank().GetCash() << std::endl;
        //выводить сколько еще денег на bankAccount
        std::cout << "Remaining money on accounts: " << simulation.GetBank().GetAccountsBalance() << std::endl;

        if (args.initialCash != simulation.GetBank().GetAccountsBalance() + simulation.GetBank().GetCash())
        {
            std::cerr << "Not enough" << std::endl;
        } else
        {
            std::cout << "Ravno" << std::endl;
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
#include <iostream>
#include "src/AtomicMax.h"

int main()
{
    try
    {
        AtomicMax<int> maxValue(0);

        std::cout << "Initial max value: " << maxValue.GetValue() << std::endl;

        maxValue.Update(10);
        std::cout << "After update with 10: " << maxValue.GetValue() << std::endl;

        maxValue.Update(5);
        std::cout << "After update with 5: " << maxValue.GetValue() << std::endl;

        maxValue.Update(15);
        std::cout << "After update with 15: " << maxValue.GetValue() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
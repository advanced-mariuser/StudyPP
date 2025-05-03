#include <iostream>
#include "src/TicketOffice.h"

int main()
{
    try
    {
        TicketOffice office(100);

        std::cout << "Initial tickets: " << office.GetTicketsLeft() << std::endl;

        int sold = office.SellTickets(30);
        std::cout << "Sold " << sold << " tickets" << std::endl;
        std::cout << "Remaining tickets: " << office.GetTicketsLeft() << std::endl;

        sold = office.SellTickets(80);
        std::cout << "Sold " << sold << " tickets" << std::endl;
        std::cout << "Remaining tickets: " << office.GetTicketsLeft() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
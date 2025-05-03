#include "TicketOffice.h"

TicketOffice::TicketOffice(int numTickets) : m_numTickets(numTickets)
{
    if (numTickets < 0)
    {
        throw std::invalid_argument("Initial ticket count cannot be negative");
    }
}

int TicketOffice::SellTickets(int ticketsToBuy)
{
    if (ticketsToBuy <= 0)
    {
        throw std::invalid_argument("ticketsToBuy must be positive");
    }

    int currentTickets = m_numTickets.load(std::memory_order_acquire);
    int newTickets;
    int ticketsSold;
    /*
    Цикл нужен, потому что между моментом, когда мы прочитали currentTickets и моментом попытки обмена,
    другой поток мог изменить значение. Если это произошло, compare_exchange_weak вернёт false,
    и мы попробуем снова с актуальным значением
    */
    do
    {
        if (currentTickets == 0)
        {
            return 0;
        }

        ticketsSold = (currentTickets >= ticketsToBuy) ? ticketsToBuy : currentTickets;
        newTickets = currentTickets - ticketsSold;

        //пересмотреть лекцию что делает acquire и release
    } while (!m_numTickets.compare_exchange_weak(
            currentTickets,
            newTickets,
            std::memory_order_release,
            std::memory_order_relaxed));

    // memory_order_release - Гарантирует, что все предыдущие операции (как атомарные, так и неатомарные) в этом потоке
    // будут видны другим потокам, которые выполняют load с memory_order_acquire на той же переменной
    // Всё, что было до release, гарантированно произойдёт раньше

    // std::memory_order_relaxed - Не дает никаких гарантий о порядке выполнения операций

    //memory_order_acquire
    // Все операции после acquire-загрузки в этом потоке будут видны позже, чем эта загрузка
    return ticketsSold;
}

int TicketOffice::GetTicketsLeft() const noexcept
{
    return m_numTickets.load(std::memory_order_acquire);
}
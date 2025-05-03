#pragma once

#include <atomic>
#include <stdexcept>

class TicketOffice
{
public:
    explicit TicketOffice(int numTickets);

    TicketOffice(const TicketOffice&) = delete;

    TicketOffice& operator=(const TicketOffice&) = delete;

    /**
     * Выполняет продажу билета.
     * Возвращает количество фактически проданных билетов.
     * Если ticketsToBuy <= 0, выбрасывается исключение std::invalid_argument
     */
    int SellTickets(int ticketsToBuy);

    int GetTicketsLeft() const noexcept;

private:
    // Количество билетов должно быть атомарным
    std::atomic<int> m_numTickets;
};
#ifndef BANK_BARTLISA_H
#define BANK_BARTLISA_H

#include "BaseCitizen.h"
#include "Apu.h"
#include <iostream>
#include <atomic>

class BartLisa : public BaseCitizen
{
private:
    Apu& m_apu;
    std::atomic<Money> m_cash; // Наличные деньги у Барта и Лизы

public:
    explicit BartLisa(Apu& apu) : m_apu(apu), m_cash(0) {}

    void Run() override
    {
        if (m_cash > 0)
        {
            try
            {
                m_apu.AddCash(m_cash);
                SafePrint("BartLisa give cash to Apu.\n");
            }
            catch (const BankOperationError& e)
            {
                std::string error = "BartLisa: Failed to deposit cash: ";
                error += e.what();
                error += "\n";
                SafePrintError(error);
            }
        }
    }

    void AddCash(Money amount)
    {
        if (amount > 0)
        {
            m_cash += amount;
        }
    }

    Money GetCash() const
    {
        return m_cash.load();
    }

    bool StealCash(Money amount)
    {
        if (amount > 0 && m_cash >= amount)
        {
            m_cash -= amount;
            return true;
        }
        return false;
    }
};

#endif //BANK_BARTLISA_H
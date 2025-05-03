#ifndef BANK_BARTLISA_H
#define BANK_BARTLISA_H

#include "BaseCitizen.h"
#include "Apu.h"
#include <iostream>

class BartLisa : public BaseCitizen
{
private:
    Apu& m_apu;

public:
    explicit BartLisa(Apu& apu) : m_apu(apu) {}

    void Run() override{}

    void AddCash(Money amount)
    {
        if (amount > 0)
        {
            try
            {
                m_apu.AddCash(amount);
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
};

#endif //BANK_BARTLISA_H
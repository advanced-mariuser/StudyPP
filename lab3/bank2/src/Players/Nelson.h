#ifndef BANK_NELSON_H
#define BANK_NELSON_H

#include "BaseCitizen.h"
#include "BartLisa.h"
#include <iostream>

class Nelson : public BaseCitizen
{
private:
    BartLisa& m_bartLisa;
    Apu& m_apu;

public:
    Nelson(BartLisa& bartLisa, Apu& apu)
            : m_bartLisa(bartLisa), m_apu(apu) {}

    void Run() override
    {
        std::string start = "Start Nelson: ";
        start += std::to_string(m_bartLisa.GetCash());
        start += "\n";
        SafePrint(start);

        // Нельсон ворует наличные у Барта и Лизы
        Money stolenAmount = NELSON_STEAL_AMOUNT;
        if (m_bartLisa.StealCash(stolenAmount))
        {
            m_apu.AddCash(stolenAmount);
            SafePrint("Nelson stole cash from BartLisa and gave it to Apu.\n");
        } else
        {
            SafePrintError("Nelson: Failed to steal cash from BartLisa (insufficient cash)\n");
        }

        std::string end = "End Nelson: ";
        end += std::to_string(m_bartLisa.GetCash());
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_NELSON_H
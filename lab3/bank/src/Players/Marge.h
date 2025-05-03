#ifndef BANK_MARGE_H
#define BANK_MARGE_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include <iostream>

class Marge : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_margeAccount;
    AccountId m_apuAccount;

public:
    Marge(Bank& bank, AccountId margeAccount, AccountId apuAccount)
            : m_bank(bank), m_margeAccount(margeAccount), m_apuAccount(apuAccount) {}

    void Run() override
    {
        std::string start = "Start Marge: ";
        start += std::to_string(m_bank.GetAccountBalance(m_margeAccount));
        start += "\n";
        SafePrint(start);

        // Мардж покупает продукты у Апу
        if (!m_bank.TrySendMoney(m_margeAccount, m_apuAccount, MARGE_PAY_APU_AMOUNT))
        {
            SafePrintError("Marge: Failed to buy groceries from Apu (insufficient funds or invalid account)\n");
        }

        std::string end = "End Marge: ";
        end += std::to_string(m_bank.GetAccountBalance(m_margeAccount));
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_MARGE_H
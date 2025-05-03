#ifndef BANK_BURNS_H
#define BANK_BURNS_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include <iostream>

class Burns : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_burnsAccount;
    AccountId m_homerAccount;

public:
    Burns(Bank& bank, AccountId burnsAccount, AccountId homerAccount)
            : m_bank(bank), m_burnsAccount(burnsAccount), m_homerAccount(homerAccount) {}

    void Run() override
    {
        std::string start = "Start Burns: ";
        start += std::to_string(m_bank.GetAccountBalance(m_burnsAccount));
        start += "\n";
        SafePrint(start);

        // Мистер Бернс платит зарплату Гомеру
        if (!m_bank.TrySendMoney(m_burnsAccount, m_homerAccount, BURNS_PAY_HOMER_AMOUNT))
        {
            SafePrintError("Burns: Failed to pay Homer's salary (insufficient funds or invalid account)\n");
        }

        std::string end = "End Burns: ";
        end += std::to_string(m_bank.GetAccountBalance(m_burnsAccount));
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_BURNS_H
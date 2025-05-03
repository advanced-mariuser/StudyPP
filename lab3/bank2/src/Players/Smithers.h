#ifndef BANK_SMITTERS_H
#define BANK_SMITTERS_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include <iostream>

class Smithers : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_smithersAccount;
    AccountId m_burnsAccount;
    AccountId m_apuAccount;

public:
    Smithers(Bank& bank, AccountId smithersAccount, AccountId burnsAccount, AccountId apuAccount)
            : m_bank(bank), m_smithersAccount(smithersAccount), m_burnsAccount(burnsAccount), m_apuAccount(apuAccount) {}

    void Run() override
    {
        std::string start = "Start Smithers: ";
        start += std::to_string(m_bank.GetAccountBalance(m_smithersAccount));
        start += "\n";
        SafePrint(start);

        // Смиттерс получает зарплату от Бернса
        if (!m_bank.TrySendMoney(m_burnsAccount, m_smithersAccount, SMITHERS_SALARY_AMOUNT))
        {
            SafePrintError("Smithers: Failed to receive salary from Burns (insufficient funds or invalid account)\n");
        }

        // Смиттерс покупает продукты у Апу
        if (!m_bank.TrySendMoney(m_smithersAccount, m_apuAccount, SMITHERS_SPEND_AMOUNT))
        {
            SafePrintError("Smithers: Failed to buy groceries from Apu (insufficient funds or invalid account)\n");
        }

        // Смиттерс закрывает и открывает счет
        Money balance = m_bank.CloseAccount(m_smithersAccount);
        m_smithersAccount = m_bank.OpenAccount();
        m_bank.DepositMoney(m_smithersAccount, balance);

        std::string end = "End Smithers: ";
        end += std::to_string(m_bank.GetAccountBalance(m_smithersAccount));
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_SMITTERS_H
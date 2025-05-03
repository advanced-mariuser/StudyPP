#ifndef BANK_APU_H
#define BANK_APU_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include <iostream>

class Apu : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_apuAccount;
    AccountId m_burnsAccount;

public:
    Apu(Bank& bank, AccountId apuAccount, AccountId burnsAccount)
            : m_bank(bank), m_apuAccount(apuAccount), m_burnsAccount(burnsAccount) {}

    void Run() override
    {
        std::string start = "Start Apu: ";
        start += std::to_string(m_bank.GetAccountBalance(m_apuAccount));
        start += "\n";
        SafePrint(start);

        // Апу платит за электроэнергию
        if (!m_bank.TrySendMoney(m_apuAccount, m_burnsAccount, APU_PAY_ELECTRICITY_AMOUNT))
        {
            SafePrintError("Apu: Failed to pay for electricity (insufficient funds or invalid account)\n");
        }

        std::string end = "End Apu: ";
        end += std::to_string(m_bank.GetAccountBalance(m_apuAccount));
        end += "\n\n";
        SafePrint(end);
    }

    void AddCash(Money amount)
    {
        if (amount > 0)
        {
            try
            {
                m_bank.DepositMoney(m_apuAccount, amount);
                SafePrint("Apu deposited cash to bank account.\n");
            }
            catch (const BankOperationError& e)
            {
                std::string error = "Apu: Failed to deposit cash: ";
                error += e.what();
                error += "\n";
                SafePrintError(error);
            }
        }
    }
};

#endif //BANK_APU_H
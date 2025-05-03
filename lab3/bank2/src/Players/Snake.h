#ifndef BANK_SNAKE_H
#define BANK_SNAKE_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include <iostream>

class Snake : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_homerAccount;
    AccountId m_snakeAccount;
    AccountId m_apuAccount;

public:
    Snake(Bank& bank, AccountId homerAccount, AccountId snakeAccount, AccountId apuAccount)
            : m_bank(bank), m_homerAccount(homerAccount), m_snakeAccount(snakeAccount), m_apuAccount(apuAccount) {}

    void Run() override
    {
        std::string start = "Start Snake: ";
        start += std::to_string(m_bank.GetAccountBalance(m_snakeAccount));
        start += "\n";
        SafePrint(start);

        // Змей взламывает счет Гомера и переводит деньги на свой счет
        Money stolenAmount = SNAKE_HACK_AMOUNT;
        if (!m_bank.TrySendMoney(m_homerAccount, m_snakeAccount, stolenAmount))
        {
            SafePrintError("Snake: Failed to hack Homer's account (insufficient funds or invalid account)\n");
        }

        // Змей покупает продукты у Апу
        if (!m_bank.TrySendMoney(m_snakeAccount, m_apuAccount, SNAKE_SPEND_AMOUNT))
        {
            SafePrintError("Snake: Failed to buy groceries from Apu (insufficient funds or invalid account)\n");
        }

        std::string end = "End Snake: ";
        end += std::to_string(m_bank.GetAccountBalance(m_snakeAccount));
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_SNAKE_H
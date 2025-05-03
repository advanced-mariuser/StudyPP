#ifndef BANK_HOMER_H
#define BANK_HOMER_H

#include "../Bank/Bank.h"
#include "BaseCitizen.h"
#include "BartLisa.h"

class Homer : public BaseCitizen
{
private:
    Bank& m_bank;
    AccountId m_homerAccount;
    AccountId m_margeAccount;
    AccountId m_burnsAccount;
    BartLisa& m_bartLisa;
public:
    Homer(Bank& bank, AccountId homerAccount, AccountId margeAccount, AccountId burnsAccount,
          BartLisa& bartLisa)
            : m_bank(bank), m_homerAccount(homerAccount), m_margeAccount(margeAccount),
              m_burnsAccount(burnsAccount), m_bartLisa(bartLisa) {}

    void Run() override
    {
        std::string start = "Start Homer: ";
        start += std::to_string(m_bank.GetAccountBalance(m_homerAccount));
        start += "\n";
        SafePrint(start);

        if (!m_bank.TrySendMoney(m_homerAccount, m_margeAccount, HOMER_SEND_TO_MARGE_AMOUNT))
        {
            SafePrintError("Homer: Failed to send money to Marge (insufficient funds or invalid account)\n");
        }

        if (!m_bank.TrySendMoney(m_homerAccount, m_burnsAccount, HOMER_PAY_ELECTRICITY_AMOUNT))
        {
            SafePrintError("Homer: Failed to pay for electricity (insufficient funds or invalid account)\n");
        }

        Money amount = HOMER_WITHDRAW_AMOUNT;

        if (!m_bank.TryWithdrawMoney(m_homerAccount, amount))
        {
            SafePrintError("Homer: Failed to withdraw cash (insufficient funds or invalid account)\n");
        } else
        {
            m_bartLisa.AddCash(HOMER_WITHDRAW_AMOUNT);
        }

        std::string end = "End Homer: ";
        end += std::to_string(m_bank.GetAccountBalance(m_homerAccount));
        end += "\n\n";
        SafePrint(end);
    }
};

#endif //BANK_HOMER_H

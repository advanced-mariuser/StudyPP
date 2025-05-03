#include "Bank.h"

// Конструктор
Bank::Bank(Money initialCash) : m_cash(initialCash), m_operationsCount(0), m_nextAccountId(1)
{
    if (initialCash < 0)
    {
        throw BankOperationError("Initial cash cannot be negative");
    }
}

// Получение количества операций
unsigned long long Bank::GetOperationsCount() const
{
    return m_operationsCount.load();
}

// Перевод денег между счетами
void Bank::SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
{
    if (!TrySendMoney(srcAccountId, dstAccountId, amount))
    {
        throw BankOperationError("Insufficient funds");
    }
}

// Попытка перевода денег между счетами
bool Bank::TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount)
{
    if (amount < 0)
    {
        throw std::out_of_range("Amount cannot be negative");
    }

    if (srcAccountId < dstAccountId)
    {
        std::unique_lock<std::shared_mutex> lockSrc(m_accountMutexes[srcAccountId]);
        std::unique_lock<std::shared_mutex> lockDst(m_accountMutexes[dstAccountId]);
    }
    else
    {
        std::unique_lock<std::shared_mutex> lockDst(m_accountMutexes[dstAccountId]);
        std::unique_lock<std::shared_mutex> lockSrc(m_accountMutexes[srcAccountId]);
    }
    m_operationsCount++;

    //написать функцию которая возвращает итератор на account
    auto srcAccountIt = GetAccountIterator(srcAccountId);
    auto dstAccountIt = GetAccountIterator(dstAccountId);

    if (srcAccountIt == m_accounts.end() || dstAccountIt == m_accounts.end())
    {
        throw BankOperationError("Invalid account ID");
    }

    if (srcAccountIt->second < amount)
    {
        return false;
    }

    srcAccountIt->second -= amount;
    dstAccountIt->second += amount;
    return true;
}

// Получение количества наличных денег
Money Bank::GetCash() const
{
    std::shared_lock<std::shared_mutex> lock(m_cashMutex);
    return m_cash;
}

Money Bank::GetAccountsBalance() const
{
    Money totalBalance = 0;
    for (const auto& [accountId, _] : m_accounts)
    {
        std::shared_lock<std::shared_mutex> lock(m_accountMutexes.at(accountId));
        totalBalance += m_accounts.at(accountId);
    }

    return totalBalance;
}

// Получение баланса счёта
Money Bank::GetAccountBalance(AccountId accountId) const
{
    std::shared_lock<std::shared_mutex> lock(m_accountMutexes.at(accountId));

    auto accountIt = GetAccountIterator(accountId);
    if (accountIt == m_accounts.end())
    {
        throw BankOperationError("Account not found");
    }

    return accountIt->second;
}

// Снятие денег со счёта
void Bank::WithdrawMoney(AccountId accountId, Money amount)
{
    if (!TryWithdrawMoney(accountId, amount))
    {
        throw BankOperationError("Insufficient funds");
    }
}

// Попытка снятия денег со счёта
bool Bank::TryWithdrawMoney(AccountId accountId, Money amount)
{
    if (amount < 0)
    {
        throw std::out_of_range("Amount cannot be negative");
    }

    std::unique_lock<std::shared_mutex> lockAccount(m_accountMutexes[accountId]);
    std::unique_lock<std::shared_mutex> lockCash(m_cashMutex);

    m_operationsCount++;

    auto accountIt = GetAccountIterator(accountId);
    if (accountIt == m_accounts.end())
    {
        throw BankOperationError("Account not found");
    }

    if (accountIt->second < amount)
    {
        return false;
    }

    accountIt->second -= amount;
    m_cash += amount;
    return true;
}

// Внесение денег на счёт
void Bank::DepositMoney(AccountId accountId, Money amount)
{
    if (amount < 0)
    {
        throw std::out_of_range("Amount cannot be negative");
    }

    std::unique_lock<std::shared_mutex> lockCash(m_cashMutex);
    std::unique_lock<std::shared_mutex> lockAccount(m_accountMutexes[accountId]);

    m_operationsCount++;

    auto accountIt = GetAccountIterator(accountId);
    if (accountIt == m_accounts.end())
    {
        throw BankOperationError("Account not found");
    }

    if (m_cash < amount)
    {
        std::string errorMessage = "Insufficient cash. Available cash: ";
        errorMessage += std::to_string(m_cash);
        errorMessage += ", required: ";
        errorMessage += std::to_string(amount);
        throw BankOperationError(errorMessage);
    }

    accountIt->second += amount;
    m_cash -= amount;
}

// Открытие нового счёта
AccountId Bank::OpenAccount()
{
    std::unique_lock<std::shared_mutex> lockCash(m_cashMutex);
    std::unique_lock<std::shared_mutex> lockAccounts(m_accountMutexes[m_nextAccountId]);

    m_operationsCount++;

    AccountId newAccountId = m_nextAccountId++;
    m_accounts[newAccountId] = 0;
    return newAccountId;
}

// Закрытие счёта
Money Bank::CloseAccount(AccountId accountId)
{
    std::unique_lock<std::shared_mutex> lockAccount(m_accountMutexes[accountId]);
    std::unique_lock<std::shared_mutex> lockCash(m_cashMutex);

    m_operationsCount++;

    auto accountIt = GetAccountIterator(accountId);
    if (accountIt == m_accounts.end())
    {
        throw BankOperationError("Account not found");
    }

    Money balance = accountIt->second;
    m_cash += balance;
    m_accounts.erase(accountIt);
    return balance;
}

//=======================================================================

std::unordered_map<AccountId, Money>::iterator Bank::GetAccountIterator(AccountId accountId)
{
    return m_accounts.find(accountId);
}

std::unordered_map<AccountId, Money>::const_iterator Bank::GetAccountIterator(AccountId accountId) const
{
    return m_accounts.find(accountId);
}
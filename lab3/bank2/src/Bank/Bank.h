#ifndef BANK_BANK_H
#define BANK_BANK_H

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <stdexcept>
#include <iostream>

using AccountId = unsigned long long;
using Money = long long;

class BankOperationError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Bank
{
private:
    std::unordered_map<AccountId, Money> m_accounts; // Хранение счетов и их балансов
    mutable std::unordered_map<AccountId, std::shared_mutex> m_accountMutexes; // Мьютексы для каждого счета
    Money m_cash; // Наличные деньги в обороте
    std::atomic<unsigned long long> m_operationsCount; // Счётчик операций
    mutable std::shared_mutex m_cashMutex;// Мьютекс для наличных
    AccountId m_nextAccountId; // Счётчик для генерации уникальных ID счетов

public:
    // Конструктор
    explicit Bank(Money initialCash);

    // Удаление конструктора копирования и оператора присваивания
    Bank(const Bank &) = delete;

    Bank &operator=(const Bank &) = delete;

    // Методы
    unsigned long long GetOperationsCount() const;

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);

    bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);

    Money GetCash() const;

    Money GetAccountBalance(AccountId accountId) const;

    void WithdrawMoney(AccountId accountId, Money amount);

    bool TryWithdrawMoney(AccountId accountId, Money amount);

    void DepositMoney(AccountId accountId, Money amount);

    AccountId OpenAccount();

    Money CloseAccount(AccountId accountId);

    std::unordered_map<AccountId, Money>::iterator GetAccountIterator(AccountId accountId);

    std::unordered_map<AccountId, Money>::const_iterator GetAccountIterator(AccountId accountId) const;

    Money GetAccountsBalance() const;
};

#endif //BANK_BANK_H

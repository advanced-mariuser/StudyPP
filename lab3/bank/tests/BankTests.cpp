#include "../src/Bank/Bank.h"
#include "catch2/catch_test_macros.hpp"
#include <sstream>

TEST_CASE("Bank operations with edge cases", "[Bank]")
{
    auto createBank = []()
    { return Bank(1000); };

    SECTION("Initial cash cannot be negative")
    {
        REQUIRE_THROWS_AS(Bank(-100), BankOperationError);
    }

    SECTION("Open account")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        REQUIRE(bank.GetAccountBalance(id) == 0);
    }

    SECTION("Deposit money to account - edge cases")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();

        bank.DepositMoney(id, 1000);
        REQUIRE(bank.GetAccountBalance(id) == 1000);
        REQUIRE(bank.GetCash() == 0);

        REQUIRE_THROWS_AS(bank.DepositMoney(id, 1), BankOperationError);

        Bank bank2 = createBank();
        AccountId id2 = bank2.OpenAccount();
        REQUIRE_THROWS_AS(bank2.DepositMoney(id2, 1001), BankOperationError);
    }

    SECTION("Deposit money - amount edge cases")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();

        REQUIRE_NOTHROW(bank.DepositMoney(id, 0)); // Должно пройти успешно
        REQUIRE(bank.GetAccountBalance(id) == 0);  // Баланс не изменился

        REQUIRE_THROWS_AS(bank.DepositMoney(id, -1), std::out_of_range);
    }

    SECTION("Deposit negative amount")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        REQUIRE_THROWS_AS(bank.DepositMoney(id, -100), std::out_of_range);
    }

    SECTION("Withdraw money from account - edge cases")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);

        bank.WithdrawMoney(id, 500);
        REQUIRE(bank.GetAccountBalance(id) == 0);
        REQUIRE(bank.GetCash() == 1000); // 1000 + 500

        REQUIRE_THROWS_AS(bank.WithdrawMoney(id, 1), BankOperationError);

        bank.DepositMoney(id, 499);
        REQUIRE_THROWS_AS(bank.WithdrawMoney(id, 500), BankOperationError);
    }

    SECTION("Withdraw money - amount edge cases")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);

        REQUIRE_NOTHROW(bank.WithdrawMoney(id, 0)); // Должно пройти успешно
        REQUIRE(bank.GetAccountBalance(id) == 500); // Баланс не изменился

        REQUIRE_THROWS_AS(bank.WithdrawMoney(id, -1), std::out_of_range);
    }


    SECTION("Withdraw negative amount")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        REQUIRE_THROWS_AS(bank.WithdrawMoney(id, -100), std::out_of_range);
    }

    SECTION("Send money between accounts - edge cases")
    {
        Bank bank = createBank();
        AccountId id1 = bank.OpenAccount();
        AccountId id2 = bank.OpenAccount();
        bank.DepositMoney(id1, 500);

        bank.SendMoney(id1, id2, 500);
        REQUIRE(bank.GetAccountBalance(id1) == 0);
        REQUIRE(bank.GetAccountBalance(id2) == 500);

        REQUIRE_THROWS_AS(bank.SendMoney(id1, id2, 1), BankOperationError);

        bank.DepositMoney(id1, 499);
        REQUIRE_THROWS_AS(bank.SendMoney(id1, id2, 500), BankOperationError);
    }

    SECTION("Send money - amount edge cases")
    {
        Bank bank = createBank();
        AccountId id1 = bank.OpenAccount();
        AccountId id2 = bank.OpenAccount();
        bank.DepositMoney(id1, 500);

        REQUIRE_NOTHROW(bank.SendMoney(id1, id2, 0)); // Должно пройти успешно
        REQUIRE(bank.GetAccountBalance(id1) == 500);  // Баланс не изменился
        REQUIRE(bank.GetAccountBalance(id2) == 0);    // Баланс не изменился

        REQUIRE_THROWS_AS(bank.SendMoney(id1, id2, -1), std::out_of_range);
    }

    SECTION("Close account with balance - edge cases")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);

        Money balance = bank.CloseAccount(id);
        REQUIRE(balance == 500);
        REQUIRE(bank.GetCash() == 1000);

        REQUIRE_THROWS_AS(bank.CloseAccount(999), BankOperationError);
    }

    SECTION("Check operations count")
    {
        Bank bank = createBank();
        AccountId id = bank.OpenAccount();
        bank.DepositMoney(id, 500);
        bank.WithdrawMoney(id, 200);
        REQUIRE(bank.GetOperationsCount() == 3);
    }
}
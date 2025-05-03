#ifndef BANK_SIMULATION_H
#define BANK_SIMULATION_H

#include "Bank/Bank.h"
#include "Players/players.h"
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>
#include <functional>

class Simulation;

std::atomic<bool> g_stopSimulation{false};

class Simulation
{
private:
    Bank m_bank;
    bool m_parallel;

    AccountId m_homerAccount{};
    AccountId m_margeAccount{};
    AccountId m_apuAccount{};
    AccountId m_burnsAccount{};

    void InitializeAccounts()
    {
        m_homerAccount = m_bank.OpenAccount();
        m_margeAccount = m_bank.OpenAccount();
        m_apuAccount = m_bank.OpenAccount();
        m_burnsAccount = m_bank.OpenAccount();

        Money initialCashPerCitizen = m_bank.GetCash() / 4;

        m_bank.DepositMoney(m_homerAccount, initialCashPerCitizen);
        m_bank.DepositMoney(m_margeAccount, initialCashPerCitizen);
        m_bank.DepositMoney(m_apuAccount, initialCashPerCitizen);
        m_bank.DepositMoney(m_burnsAccount, initialCashPerCitizen);
    }

    std::vector<std::unique_ptr<ICitizen>> CreatePlayers()
    {
        std::vector<std::unique_ptr<ICitizen>> citizens;

        auto apu = std::make_unique<Apu>(m_bank, m_apuAccount, m_burnsAccount);
        auto bartLisa = std::make_unique<BartLisa>(*apu);
        auto homer = std::make_unique<Homer>(m_bank, m_homerAccount, m_margeAccount,
                                             m_burnsAccount, *bartLisa);
        auto marge = std::make_unique<Marge>(m_bank, m_margeAccount, m_apuAccount);
        auto burns = std::make_unique<Burns>(m_bank, m_burnsAccount, m_homerAccount);

        citizens.push_back(std::move(homer));
        citizens.push_back(std::move(marge));
        citizens.push_back(std::move(bartLisa));
        citizens.push_back(std::move(apu));
        citizens.push_back(std::move(burns));

        return citizens;
    }

    void RunActorsParallel(const std::vector<std::unique_ptr<ICitizen>>& citizens)
    {
        std::vector<std::jthread> threads;

        auto runActor = [this](const std::unique_ptr<ICitizen>& actor)
        {
            while (!g_stopSimulation.load())
            {
                actor->Run();
            }
        };

        threads.reserve(citizens.size());
        for (auto& citizen: citizens)
        {
            threads.emplace_back(runActor, std::ref(citizen));
        }
    }

    static void RunActorsSequential(const std::vector<std::unique_ptr<ICitizen>>& citizens)
    {
        while (!g_stopSimulation.load())
        {
            for (auto& citizen: citizens)
            {
                citizen->Run();
            }
        }
    }

    static void HandleSignal(int signal)
    {
        if (signal == SIGINT || signal == SIGTERM)
        {
            g_stopSimulation.store(true);
        }
    }

public:
    Simulation(bool parallel, Money initialCash)
            : m_parallel(parallel), m_bank(initialCash) {}

    Bank& GetBank()
    {
        return m_bank;
    }

    void RunSimulation()
    {
        std::signal(SIGINT, HandleSignal);
        std::signal(SIGTERM, HandleSignal);

        InitializeAccounts();

        auto citizens = CreatePlayers();

        if (m_parallel)
        {
            RunActorsParallel(citizens);
        } else
        {
            RunActorsSequential(citizens);
        }
    }
};

#endif // BANK_SIMULATION_H
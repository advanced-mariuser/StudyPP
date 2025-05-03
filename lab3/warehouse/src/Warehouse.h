#ifndef WAREHOUSE_WAREHOUSE_H
#define WAREHOUSE_WAREHOUSE_H

#include <mutex>
#include <condition_variable>

class Warehouse
{
private:
    int m_capacity;
    int m_stock;
    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::atomic<bool>& m_stop;

public:
    explicit Warehouse(int capacity, std::atomic<bool>& stop)
            : m_capacity(capacity), m_stock(0), m_stop(stop) {}

    bool AddGoods(int amount)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        //как будто m_conditionVariable вступает в конфликт с булевым флагом
        auto canAddGoods = [this, amount] { return m_stock + amount <= m_capacity; };
        m_conditionVariable.wait(lock, canAddGoods);

        if (m_stop.load()) return false;

        m_stock += amount;
        m_conditionVariable.notify_all();
        return true;
    }

    bool TakeGoods(int amount)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto canTakeGoods = [this, amount] { return m_stock >= amount; };
        m_conditionVariable.wait(lock, canTakeGoods);

        if (m_stop.load()) return false;

        m_stock -= amount;
        m_conditionVariable.notify_all();
        return true;
    }

    int GetStock()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stock;
    }
};

#endif //WAREHOUSE_WAREHOUSE_H

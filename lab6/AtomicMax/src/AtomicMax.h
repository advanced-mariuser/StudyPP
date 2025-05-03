#pragma once
#include <atomic>
#include <type_traits>

template <typename T>
class AtomicMax
{
    static_assert(std::is_arithmetic<T>::value,
                  "AtomicMax requires arithmetic type");

public:
    explicit AtomicMax(T value) : m_value(value)
    {
    }

    void Update(T newValue) noexcept
    {
        T current = m_value.load(std::memory_order_acquire);
        while (newValue > current)
        {
            if (m_value.compare_exchange_weak(
                    current,
                    newValue,
                    std::memory_order_release,
                    std::memory_order_relaxed))
            {
                break;
            }
        }
    }

    T GetValue() const noexcept
    {
        return m_value.load(std::memory_order_acquire);
    }

    AtomicMax(const AtomicMax&) = delete;
    AtomicMax& operator=(const AtomicMax&) = delete;

private:
    std::atomic<T> m_value;
};
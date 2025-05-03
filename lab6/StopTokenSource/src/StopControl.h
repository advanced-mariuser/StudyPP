#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

//Безопасная остановка потоков

class StopToken;

class StopSource
{
public:
    StopSource();

    ~StopSource() = default;

    StopSource(const StopSource&) = delete;

    StopSource& operator=(const StopSource&) = delete;

    void RequestStop() noexcept;

    StopToken GetToken() noexcept;

    bool StopRequested() const noexcept;

private:
    friend class StopToken;

    std::shared_ptr<std::atomic<bool>> m_stopFlag;
    std::shared_ptr<std::mutex> m_mutex;
    std::shared_ptr<std::condition_variable> m_stopCondition;
};

class StopToken
{
public:
    StopToken() = default;

    ~StopToken() = default;

    StopToken(const StopToken&) = default;

    StopToken& operator=(const StopToken&) = default;

    bool StopRequested() const noexcept;

    void WaitForStop() const;

private:
    friend class StopSource;

    explicit StopToken(std::shared_ptr<std::atomic<bool>> stopFlag,
                       std::shared_ptr<std::mutex> mutex,
                       std::shared_ptr<std::condition_variable> cv);

    std::shared_ptr<std::atomic<bool>> m_stopFlag;
    std::shared_ptr<std::mutex> m_mutex;
    std::shared_ptr<std::condition_variable> m_stopCondition;
};
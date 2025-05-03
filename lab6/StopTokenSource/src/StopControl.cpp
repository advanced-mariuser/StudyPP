#include "StopControl.h"
#include <stdexcept>

StopSource::StopSource() :
        m_stopFlag(std::make_shared<std::atomic<bool>>(false)),
        m_mutex(std::make_shared<std::mutex>()),
        m_stopCondition(std::make_shared<std::condition_variable>())
{
}

void StopSource::RequestStop() noexcept
{
    if (!m_stopFlag->exchange(true))
    {
        std::lock_guard<std::mutex> lock(*m_mutex);
        m_stopCondition->notify_all();
    }
}

StopToken StopSource::GetToken() noexcept
{
    return StopToken(m_stopFlag, m_mutex, m_stopCondition);
}

bool StopSource::StopRequested() const noexcept
{
    return m_stopFlag->load();
}

//================================================================================

StopToken::StopToken(std::shared_ptr<std::atomic<bool>> stopFlag,
                     std::shared_ptr<std::mutex> mutex,
                     std::shared_ptr<std::condition_variable> cv) :
        m_stopFlag(std::move(stopFlag)),
        m_mutex(std::move(mutex)),
        m_stopCondition(std::move(cv))
{
}

bool StopToken::StopRequested() const noexcept
{
    return m_stopFlag && m_stopFlag->load();
}

//проверить что будет когда stopsource будет разрушен
void StopToken::WaitForStop() const
{
    if (!m_stopFlag)
    {
        throw std::runtime_error("StopToken is empty");
    }

    std::unique_lock<std::mutex> lock(*m_mutex);
    m_stopCondition->wait(lock, [this]
    { return m_stopFlag->load(); });
}
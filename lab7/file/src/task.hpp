#pragma once
#include <coroutine>
#include <exception>
#include <utility>

struct Task
{
    struct promise_type
    {
        std::exception_ptr m_exception;
        std::coroutine_handle<> m_continuation = nullptr;

        Task get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept
        { return {}; }

        struct FinalAwaiter
        {
            bool await_ready() noexcept
            { return false; }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept
            {
                if (h.promise().m_continuation)
                {
                    return h.promise().m_continuation;
                }
                return std::noop_coroutine();
            }

            void await_resume() noexcept
            {}
        };

        FinalAwaiter final_suspend() noexcept
        {
            return {};
        }

        void return_void()
        {}

        void unhandled_exception()
        { m_exception = std::current_exception(); }
    };

    std::coroutine_handle<promise_type> m_handle;

    Task(std::coroutine_handle<promise_type> h = nullptr) : m_handle(h)
    {}

    ~Task()
    {
        if (m_handle && m_handle.done())
        {
            m_handle.destroy();
        }
    }

    Task(Task &&other) noexcept: m_handle(std::exchange(other.m_handle, nullptr))
    {}

    Task &operator=(Task &&other) noexcept
    {
        if (this != &other)
        {
            if (m_handle && m_handle.done())
            {
                m_handle.destroy();
            }

            m_handle = std::exchange(other.m_handle, nullptr);
        }
        return *this;
    }

    Task(const Task &) = delete;

    Task &operator=(const Task &) = delete;

    bool await_ready() const noexcept
    {
        return (!m_handle || m_handle.done());
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
    {
        m_handle.promise().m_continuation = awaitingCoroutine;
        return m_handle;
    }

    void await_resume() const
    {
        if (m_handle && m_handle.promise().m_exception)
        {
            std::rethrow_exception(m_handle.promise().m_exception);
        }
    }

    void Start()
    {
        if (m_handle && !m_handle.done())
        {
            m_handle.resume();
        }
    }
};

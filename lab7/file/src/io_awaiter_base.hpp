#pragma once
#include <coroutine>

class Dispatcher;

struct IoUringAwaiterBase
{
    Dispatcher *m_dispatcher;
    std::coroutine_handle<> m_coroHandle;
    int m_result;

    IoUringAwaiterBase(Dispatcher *dispatcher) : m_dispatcher(dispatcher), m_result(0)
    {}

    bool await_ready() const noexcept
    {
        return false;
    }

    //
};
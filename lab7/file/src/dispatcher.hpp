#pragma once

#include <liburing.h>
#include <atomic>
#include <system_error>
#include <stdexcept>

struct Task;

class Dispatcher
{
public:
    // сделать приватными
    // что такое pimpl
    struct io_uring m_ring;
    std::atomic<int> m_activeOperations;

    explicit Dispatcher(unsigned entries = 64);

    ~Dispatcher();

    Dispatcher(const Dispatcher &) = delete;

    Dispatcher &operator=(const Dispatcher &) = delete;

    // сделать методы приватными которые должны быть приватными
    io_uring_sqe *GetSqe();

    void Submit();

    void ProcessCompletions();

    void Run(Task &taskToRun);
};
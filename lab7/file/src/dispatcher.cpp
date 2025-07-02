#include "dispatcher.hpp"
#include "task.hpp"
#include "io_awaiter_base.hpp"
#include <iostream>

Dispatcher::Dispatcher(unsigned entries) : m_activeOperations(0)
{
    int ret = io_uring_queue_init(entries, &m_ring, 0);
    if (ret < 0)
    {
        throw std::system_error(-ret, std::system_category(), "Ошибка io_uring_queue_init");
    }
}

Dispatcher::~Dispatcher()
{
    io_uring_queue_exit(&m_ring);
}

io_uring_sqe *Dispatcher::GetSqe()
{
    io_uring_sqe *sqe = io_uring_get_sqe(&m_ring);
    if (!sqe)
    {
        Submit();
        sqe = io_uring_get_sqe(&m_ring);
        if (!sqe)
        {
            throw std::runtime_error("Не удалось получить SQE после submit (очередь заполнена?)");
        }
    }
    m_activeOperations++;
    return sqe;
}

void Dispatcher::Submit()
{
    int ret = io_uring_submit(&m_ring);
    if (ret < 0)
    {
        throw std::system_error(-ret, std::system_category(), "Ошибка io_uring_submit");
    }
}

void Dispatcher::ProcessCompletions()
{
    io_uring_cqe *cqe;
    unsigned head;
    unsigned count = 0;

    io_uring_for_each_cqe(&m_ring, head, cqe)
    {
        count++;
        auto *awaiter = static_cast<IoUringAwaiterBase *>(io_uring_cqe_get_data(cqe));
        if (awaiter)
        {
            awaiter->m_result = cqe->res;
            if (awaiter->m_coroHandle)
            {
                awaiter->m_coroHandle.resume();
            }
        }
        m_activeOperations--;
    }
    if (count > 0)
    {
        io_uring_cq_advance(&m_ring, count);
    }
}

void Dispatcher::Run(Task &taskToRun)
{
    if (taskToRun.m_handle)
    {
        taskToRun.Start();
    }

    while ((taskToRun.m_handle && !taskToRun.m_handle.done()) || m_activeOperations > 0)
    {
        Submit();

        if (m_activeOperations > 0)
        {
            io_uring_cqe *cqe_ptr = nullptr;
            int ret = io_uring_wait_cqe(&m_ring, &cqe_ptr);
            if (ret < 0)
            {
                throw std::system_error(-ret, std::system_category(), "Ошибка io_uring_wait_cqe");
            }
        }
        ProcessCompletions();
    }

    if (taskToRun.m_handle && taskToRun.m_handle.done() && taskToRun.m_handle.promise().m_exception)
    {
        std::rethrow_exception(taskToRun.m_handle.promise().m_exception);
    }
}
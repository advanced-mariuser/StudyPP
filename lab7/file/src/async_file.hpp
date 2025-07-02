#pragma once

#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <system_error>
#include <stdexcept>
#include <filesystem>
#include <utility>

#include <liburing.h>

#include "io_awaiter_base.hpp"
#include "dispatcher.hpp"


enum class OpenMode
{
    Read,
    Write
};

class AsyncFile
{
public:
    int m_fd;

    AsyncFile() : m_fd(-1)
    {}

    explicit AsyncFile(int fd) : m_fd(fd)
    {}

    ~AsyncFile()
    {
        if (m_fd != -1)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    AsyncFile(AsyncFile &&other) noexcept: m_fd(std::exchange(other.m_fd, -1))
    {}

    AsyncFile &operator=(AsyncFile &&other) noexcept
    {
        if (this != &other)
        {
            if (m_fd != -1) ::close(m_fd);
            m_fd = std::exchange(other.m_fd, -1);
        }
        return *this;
    }

    AsyncFile(const AsyncFile &) = delete;

    AsyncFile &operator=(const AsyncFile &) = delete;

    bool IsOpen() const
    { return m_fd != -1; }

    struct ReadAwaiter : IoUringAwaiterBase
    {
        AsyncFile *m_file;
        char *m_buffer;
        size_t m_size;

        ReadAwaiter(Dispatcher *dispatcher, AsyncFile *file, char *buffer, size_t size)
                : IoUringAwaiterBase(dispatcher), m_file(file), m_buffer(buffer), m_size(size)
        {}

        void await_suspend(std::coroutine_handle<> h)
        {
            m_coroHandle = h;
            io_uring_sqe *sqe = m_dispatcher->GetSqe();
            io_uring_prep_read(sqe, m_file->m_fd, m_buffer, m_size, -1);
            io_uring_sqe_set_data(sqe, this);
        }

        int await_resume()
        {
            if (m_result < 0)
            {
                throw std::system_error(-m_result, std::system_category(), "Ошибка AsyncFile ReadAsync");
            }
            return m_result;
        }
    };

    ReadAwaiter ReadAsync(Dispatcher &dispatcher, char *buffer, size_t bufferSize)
    {
        if (m_fd == -1) throw std::runtime_error("Файл не открыт для ReadAsync");
        return ReadAwaiter(&dispatcher, this, buffer, bufferSize);
    }

    struct WriteAwaiter : IoUringAwaiterBase
    {
        AsyncFile *m_file;
        const char *m_buffer;
        size_t m_size;

        WriteAwaiter(Dispatcher *dispatcher, AsyncFile *file, const char *buffer, size_t size)
                : IoUringAwaiterBase(dispatcher), m_file(file), m_buffer(buffer), m_size(size)
        {}

        void await_suspend(std::coroutine_handle<> h)
        {
            m_coroHandle = h;
            io_uring_sqe *sqe = m_dispatcher->GetSqe();
            io_uring_prep_write(sqe, m_file->m_fd, m_buffer, m_size, -1);
            io_uring_sqe_set_data(sqe, this);
        }

        int await_resume()
        {
            if (m_result < 0)
            {
                throw std::system_error(-m_result, std::system_category(), "Ошибка AsyncFile WriteAsync");
            }
            return m_result;
        }
    };

    WriteAwaiter WriteAsync(Dispatcher &dispatcher, const char *buffer, size_t bytesToWrite)
    {
        if (m_fd == -1) throw std::runtime_error("Файл не открыт для WriteAsync");
        return WriteAwaiter(&dispatcher, this, buffer, bytesToWrite);
    }
};

struct OpenFileAwaiter : IoUringAwaiterBase
{
    std::filesystem::path m_path;
    OpenMode m_mode;

    OpenFileAwaiter(Dispatcher *dispatcher, std::filesystem::path path, OpenMode mode)
            : IoUringAwaiterBase(dispatcher), m_path(std::move(path)), m_mode(mode)
    {}

    void await_suspend(std::coroutine_handle<> h)
    {
        m_coroHandle = h;
        io_uring_sqe *sqe = m_dispatcher->GetSqe();
        int flags = 0;
        mode_t createMode = 0;
        if (m_mode == OpenMode::Read)
        {
            flags = O_RDONLY;
        } else
        {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            createMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        }
        io_uring_prep_openat(sqe, AT_FDCWD, m_path.c_str(), flags, createMode);
        io_uring_sqe_set_data(sqe, this);
    }

    AsyncFile await_resume()
    {
        if (m_result < 0)
        {
            throw std::system_error(-m_result, std::system_category(), "Ошибка AsyncOpenFile для " + m_path.string());
        }
        return AsyncFile(m_result);
    }
};

OpenFileAwaiter AsyncOpenFile(Dispatcher &dispatcher, const std::string &path, OpenMode mode)
{
    return OpenFileAwaiter(&dispatcher, std::filesystem::path(path), mode);
}
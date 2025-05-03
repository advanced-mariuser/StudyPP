#pragma once

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <functional>

template<typename T>
class LockFreeQueue
{
public:
    explicit LockFreeQueue(size_t capacity) : queue(capacity) {}

    bool push(T&& item)
    {
        return queue.push(std::move(item));
    }

    bool pop(T& item)
    {
        return queue.pop(item);
    }

    [[nodiscard]] bool empty() const
    {
        return queue.empty();
    }

private:
    boost::lockfree::queue <T> queue;
};
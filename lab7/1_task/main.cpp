#include <coroutine>
#include <exception>
#include <iostream>
#include <string>

//std::suspend_always
//Всегда приостанавливает выполнение корутины, когда встречается
//В этот момент корутина приостанавливает выполнение.
//Выполнение возобновляется только при явном возобновлении через coroutine_handle::resume()
//std::suspend_never
//Никогда не приостанавливает выполнение корутины при встрече с ней
//Корутина продолжает выполняться непрерывно
//Точка приостановки не создается

class MyTask
{
public:
    struct promise_type
    {
        std::string value;
        std::exception_ptr exception;

        MyTask get_return_object() // Вызывается при первом вызове coroutine для создания возвращаемого объекта
        {
            return MyTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept { return {}; } // Вызывается в начале выполнения корутины

        std::suspend_always final_suspend() noexcept { return {}; } // Что делать в самом конце

        void return_value(std::string val)  // Вызывается, когда co_return используется со строковым значением
        {
            value = std::move(val);
        }

        void unhandled_exception()
        {
            exception = std::current_exception();
        }
    };

    explicit MyTask(std::coroutine_handle<promise_type> handle)
            : m_handle(handle) {}

    ~MyTask()
    {
        if (m_handle)
        {
            m_handle.destroy();
        }
    }

    std::string GetResult()
    {
        if (!m_handle.done())
        {
            m_handle.resume();
        }
        if (m_handle.promise().exception)
        {
            std::rethrow_exception(m_handle.promise().exception);
        }
        return m_handle.promise().value;
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};

MyTask SimpleCoroutine()
{
    co_return "Hello from coroutine!";
}

int main()
{
    MyTask task = SimpleCoroutine();
    std::cout << task.GetResult() << std::endl;
    return EXIT_SUCCESS;
}
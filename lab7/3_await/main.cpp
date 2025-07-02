#include <coroutine>
#include <iostream>

//if (!t.await_ready()) {
//    t.await_suspend(handleOfCurrentCoroutine);
//}
//int result = t.await_resume();
struct MyAwaiter
{
    int x;
    int y;

    bool await_ready() const noexcept
    {
        return false;
    }

    //Возвращаемся в main
    void await_suspend(std::coroutine_handle<> handle) const noexcept
    {
    }

    int await_resume() const noexcept
    {
        return x + y;
    }
};

//Как сделать так чтобы MyTask мог быть awiter но чтобы у него не торчали методы await_ready await_suspend await_resume
class MyTask
{
public:
    struct promise_type
    {
        MyTask get_return_object()
        {
            return MyTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}

        void unhandled_exception() { std::terminate(); }
    };

    explicit MyTask(std::coroutine_handle<promise_type> handle) : m_handle(handle) {}

    ~MyTask() { if (m_handle) m_handle.destroy(); }

    //Корутина продолжает выполнение с точки после co_await
    void Resume()
    {
        if (m_handle && !m_handle.done())
        {
            m_handle.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};

MyTask CoroutineWithAwait(int x, int y)
{
    std::cout << "Before await\n";
    int result = co_await MyAwaiter{x, y};
    std::cout << result << "\n";
    std::cout << "After await\n";
}

int main()
{
    auto task = CoroutineWithAwait(30, 12);
    std::cout << "Before resume\n";
    task.Resume();
    std::cout << "After resume\n";
    CoroutineWithAwait(5, 10).Resume();
    std::cout << "End of main\n";
}
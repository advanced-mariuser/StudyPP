#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/init/Init.h>
#include <iostream>

using namespace folly::coro;

Task<int> asyncCompute(int x)
{
    co_await folly::futures::sleep(std::chrono::seconds(1));
    co_return x * 2;
}

Task<void> exampleTask()
{
    std::cout << "Start task\n";

    int result1 = co_await asyncCompute(21);
    std::cout << "Result 1: " << result1 << "\n";

    int result2 = co_await asyncCompute(result1);
    std::cout << "Result 2: " << result2 << "\n";
}

int main(int argc, char* argv[])
{
    // Обязательная инициализация Folly
    folly::Init init(&argc, &argv);

    // Создаем пул потоков
    folly::CPUThreadPoolExecutor executor(1);

    // Запуск корутины
    auto task = exampleTask().scheduleOn(&executor).start();

    // Ожидание завершения
    std::move(task).get();

    return EXIT_SUCCESS;
}
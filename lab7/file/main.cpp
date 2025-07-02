#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <system_error>
#include <stdexcept>
#include <clocale>

#include "task.hpp"
#include "dispatcher.hpp"
#include "async_file.hpp"

Task AsyncCopyFile(Dispatcher &dispatcher, std::string from, std::string to)
{
    std::cout << "Начинаем AsyncCopyFile для " << from << " -> " << to << std::endl;
    AsyncFile input = co_await AsyncOpenFile(dispatcher, from, OpenMode::Read);
    if (!input.IsOpen())
    {
        std::cerr << "Не удалось открыть входной файл: " << from << std::endl;
        co_return;
    }
    std::cout << "Открыт " << from << " для чтения (fd: " << input.m_fd << ")" << std::endl;

    AsyncFile output = co_await AsyncOpenFile(dispatcher, to, OpenMode::Write);
    if (!output.IsOpen())
    {
        std::cerr << "Не удалось открыть выходной файл: " << to << std::endl;
        co_return;
    }
    std::cout << "Открыт " << to << " для записи (fd: " << output.m_fd << ")" << std::endl;

    std::vector<char> buffer(1024 * 4);
    unsigned long long totalBytesCopied = 0;

    for (ssize_t bytesRead;(bytesRead = co_await input.ReadAsync(dispatcher, buffer.data(), buffer.size())) > 0;)
    {
        ssize_t bytesWritten = co_await output.WriteAsync(dispatcher, buffer.data(), bytesRead);
        if (bytesWritten != bytesRead)
        {
            std::cerr << "Короткая запись в " << to << "! Ожидалось " << bytesRead << ", записано " << bytesWritten
                      << std::endl;
            break;
        }
        totalBytesCopied += bytesWritten;
    }
    std::cout << "Завершено копирование " << from << " в " << to << ". Всего байт: " << totalBytesCopied << std::endl;
}

Task AsyncCopyTwoFiles(Dispatcher &dispatcher)
{
    std::cout << "Начинаем AsyncCopyTwoFiles." << std::endl;
    Task t1 = AsyncCopyFile(dispatcher, "a.in", "a.out");
    Task t2 = AsyncCopyFile(dispatcher, "b.in", "b.out");

    co_await t1;
    std::cout << "Задача 1 (a.in -> a.out) завершена." << std::endl;
    co_await t2;
    std::cout << "Задача 2 (b.in -> b.out) завершена." << std::endl;
    std::cout << "AsyncCopyTwoFiles завершен." << std::endl;
}

int main()
{
    std::setlocale(LC_ALL, "Russian");

    {
        std::ofstream fa("a.in");
        for (int i = 0; i < 512; ++i) fa << "Это файл A, строка " << i << " какого-то текста для проверки.\n";
        std::ofstream fb("b.in");
        for (int i = 0; i < 256; ++i) fb << "Содержимое файла B, элемент " << i << " еще немного данных.\n";
    }

    try
    {
        Dispatcher dispatcher;
        std::cout << "Диспетчер создан." << std::endl;

        Task mainTask = AsyncCopyTwoFiles(dispatcher);
        std::cout << "Главная задача (AsyncCopyTwoFiles) создана. Запускаем диспетчер..." << std::endl;

        dispatcher.Run(mainTask);

        std::cout << "Диспетчер завершил работу." << std::endl;

    }
    catch (const std::system_error &e)
    {
        std::cerr << "Системная ошибка: " << e.what() << " (код: " << e.code().value() << ")" << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Программа успешно завершена." << std::endl;
    return 0;
}
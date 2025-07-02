#pragma once

#include <pulse/simple.h>
#include <pulse/error.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>

class RadioClient
{
public:
    RadioClient(const std::string& address, int port)
            : m_address(address), m_port(port)
    {
        SetupNetwork();
        SetupAudio();
    }

    ~RadioClient()
    {
        Cleanup();
    }

    void Run()
    {
        m_networkThread = std::jthread(&RadioClient::NetworkReceive, this);
        m_audioThread = std::jthread(&RadioClient::PlayAudio, this);
    }

    void Stop()
    {
        m_running = false;
    }

private:
    std::string m_address;
    int m_port;
    pa_simple* m_pulseAudio = nullptr;
    // Дескриптор UDP-сокета
    int m_sockfd = -1;
    // Буфер для хранения полученных аудиоданных
    std::vector<int16_t> m_audioBuffer;
    // Мьютекс для синхронизации доступа к буферу
    std::mutex m_bufferMutex;
    std::atomic<bool> m_running{true};
    //Потоки
    std::jthread m_networkThread;
    std::jthread m_audioThread;

    void SetupNetwork()
    {
        //AF_INET сокет будет работать в домене IPv4
        //SOCK_DGRAM - константа, определяющая тип сокета: датаграммный (UDP)
        m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        std::cout << m_address << std::endl;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port); //// Порт в сетевом порядке байт
        //Преобразуем строку IP в бинарный формат
        if (inet_pton(AF_INET, m_address.c_str(), &addr.sin_addr) <= 0)
        {
            throw std::runtime_error("Invalid address");
        }

        //Подключаем" UDP-сокет к серверу
        if (connect(m_sockfd, (sockaddr*) &addr, sizeof(addr)))
        {
            throw std::runtime_error("Failed to connect");
        }

        // Отправляем пустой пакет серверу
        const char dummy = 0;
        send(m_sockfd, &dummy, 1, 0);
    }

    void SetupAudio()
    {
        // Описание формата аудио
        static const pa_sample_spec sampleSpec = {
                .format = PA_SAMPLE_S16LE, // 16-битный PCM, little-endian
                .rate = 44100, // Частота дискретизации
                .channels = 1
        };

        int error;
        m_pulseAudio = pa_simple_new(nullptr, "RadioClient", PA_STREAM_PLAYBACK, //вывод звука на динамики
                                     nullptr /*по умолчанию*/, "playback", &sampleSpec, nullptr, nullptr, &error);
        if (!m_pulseAudio)
        {
            throw std::runtime_error("Failed to create PulseAudio connection: " +
                                     std::string(pa_strerror(error)));
        }
    }

    void PlayAudio()
    {
        constexpr size_t bufferSize = 1024; // Размер блока
        int16_t buffer[bufferSize];
        int error;

        while (m_running)
        {
            {
                std::lock_guard<std::mutex> lock(m_bufferMutex);
                // Блокируем доступ к буферу аудио
                if (m_audioBuffer.size() >= bufferSize)
                {
                    std::copy(m_audioBuffer.begin(), m_audioBuffer.begin() + bufferSize, buffer);
                    m_audioBuffer.erase(m_audioBuffer.begin(), m_audioBuffer.begin() + bufferSize);
                } else
                {
                    std::fill(buffer, buffer + bufferSize, 0);
                }
            }

            std::cout << "[AUDIO OUT] ";
            for (size_t i = 0; i < std::min(bufferSize, size_t(8)); ++i)
                std::cout << buffer[i] << " ";
            std::cout << "..." << std::endl;

            // Отправляем аудиоданные в PulseAudio
            if (pa_simple_write(m_pulseAudio, buffer, sizeof(buffer), &error))
            {
                std::cerr << "PulseAudio write failed: " << pa_strerror(error) << std::endl;
                break;
            }
        }

        // После выхода из цикла дожидаемся полного воспроизведения буфера
        if (pa_simple_drain(m_pulseAudio, &error))
        {
            std::cerr << "PulseAudio drain failed: " << pa_strerror(error) << std::endl;
        }
    }

    void NetworkReceive()
    {
        std::vector<int16_t> buffer(1024);

        while (m_running)
        {
            // Получаем данные из сокета
            int bytes = recv(m_sockfd, buffer.data(), buffer.size() * sizeof(int16_t), 0);
            if (bytes <= 0)
            {
                break;
            }

            int samples = bytes / sizeof(int16_t);

            for (int i = 0; i < samples; i++)
            {
                std::cout << "[UDP RECV] ";
                for (int i = 0; i < std::min(samples, 8); ++i)
                    std::cout << buffer[i] << " ";
                std::cout << "..." << std::endl;

                // Преобразуем каждый сэмпл из сетевого порядка байт (big-endian) в порядок хоста
                buffer[i] = ntohs(buffer[i]);
            }

            std::lock_guard<std::mutex> lock(m_bufferMutex);
            m_audioBuffer.insert(m_audioBuffer.end(), buffer.begin(), buffer.begin() + samples);
        }
    }

    void Cleanup()
    {
        m_running = false;

        if (m_pulseAudio)
        {
            pa_simple_free(m_pulseAudio);
        }

        if (m_sockfd != -1)
        {
            close(m_sockfd);
        }
    }
};
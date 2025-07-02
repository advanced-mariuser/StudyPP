#pragma once

#include <pulse/simple.h>
#include <pulse/error.h>
#include <netinet/in.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <arpa/inet.h>

class RadioServer
{
public:
    explicit RadioServer(int port) : m_port(port)
    {
        SetupAudio();
        SetupNetwork();
    }

    ~RadioServer()
    {
        Cleanup();
    }

    void Run()
    {
        m_audioThread = std::jthread(&RadioServer::AudioCapture, this);
        m_networkThread = std::jthread(&RadioServer::NetworkSend, this);
    }

    void Stop()
    {
        m_running = false;
    }

private:
    int m_port;
    pa_simple* m_pulseAudio = nullptr;
    // Дескриптор UDP-сокета
    int m_sockfd = -1;
    // Буфер для хранения отправляемых аудиоданных
    std::vector<int16_t> m_audioBuffer;
    std::mutex m_bufferMutex;
    std::condition_variable m_bufferCv;
    std::atomic<bool> m_running{true};
    //потоки
    std::jthread m_audioThread;
    std::jthread m_networkThread;

    void SetupAudio()
    {
        static const pa_sample_spec sampleSpec = {
                .format = PA_SAMPLE_S16LE,
                .rate = 44100,
                .channels = 1
        };

        int error;
        m_pulseAudio = pa_simple_new(nullptr, "RadioServer", PA_STREAM_RECORD, //захват звука с микрофона
                                     nullptr /*устройство по умолчанию*/, "record", &sampleSpec, nullptr, nullptr,
                                     &error);
        if (!m_pulseAudio)
        {
            throw std::runtime_error("Failed to create PulseAudio connection: " +
                                     std::string(pa_strerror(error)));
        }
    }

    void SetupNetwork()
    {
        //AF_INET сокет будет работать в домене IPv4
        //SOCK_DGRAM - константа, определяющая тип сокета: датаграммный (UDP)
        m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockfd < 0)
        {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY; // Любой локальный адрес
        addr.sin_port = htons(m_port);

        // Привязываем сокет к адресу и порту
        if (bind(m_sockfd, (sockaddr*) &addr, sizeof(addr)))
        {
            throw std::runtime_error("Failed to bind socket");
        }
    }

    void AudioCapture()
    {
        constexpr size_t bufferSize = 1024;// Размер блока
        int16_t buffer[bufferSize];
        int error;

        while (m_running)
        {
            // Считываем аудиоданные с микрофона в буфер
            if (pa_simple_read(m_pulseAudio, buffer, sizeof(buffer), &error) < 0)
            {
                std::cerr << "PulseAudio read failed: " << pa_strerror(error) << std::endl;
                break;
            }

            std::cout << "[MIC] ";
            for (size_t i = 0; i < std::min(bufferSize, size_t(8)); ++i)
                std::cout << buffer[i] << " ";
            std::cout << "..." << std::endl;

            // Копируем считанные данные в общий буфер под защитой мьютекса
            std::lock_guard<std::mutex> lock(m_bufferMutex);
            m_audioBuffer.insert(m_audioBuffer.end(), buffer, buffer + bufferSize);
            m_bufferCv.notify_one();
            // Оповещаем поток отправки о новых данных
        }
    }

    //TODO: Продумать механизм когда один поток начинает генерировать данные быстрее, чем успевает отправлять другой
    //TODO: Продумать момент когда получатель и отправитель имеют разные частоты записи и воспроизведения
    void NetworkSend()
    {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);

        std::cout << "Waiting for client to connect..." << std::endl;
        // Ждём сообщение от первого клиента
        char buf[1];
        ssize_t n = recvfrom(m_sockfd, buf, sizeof(buf), 0, (sockaddr*) &clientAddr, &addrLen);
        if (n > 0)
        {
            std::cout << "Client connected from "
                      << inet_ntoa(clientAddr.sin_addr) << ":"
                      << ntohs(clientAddr.sin_port) << std::endl;
        } else
        {
            std::cerr << "Error waiting for client connection!" << std::endl;
            return;
        }

        while (m_running)
        {
            std::vector<int16_t> buffer;
            {
                // Ожидаем появления данных в буфере
                std::unique_lock<std::mutex> lock(m_bufferMutex);
                m_bufferCv.wait(lock, [this]
                {
                    return !m_running || !m_audioBuffer.empty();
                });

                if (!m_running)
                {
                    break;
                }

                buffer = std::move(m_audioBuffer);
                m_audioBuffer.clear();
            }

            for (auto& sample: buffer)
            {
                sample = htons(sample);
            }

            std::cout << "[UDP] ";
            for (size_t i = 0; i < std::min(buffer.size(), size_t(8)); ++i)
                std::cout << ntohs(buffer[i]) << " "; // выводим в хост-байт-ордере для читаемости
            std::cout << "..." << std::endl;

            sendto(m_sockfd, buffer.data(), buffer.size() * sizeof(int16_t), 0,
                   (sockaddr*) &clientAddr, addrLen);
        }
    }

    void Cleanup()
    {
        m_running = false;
        m_bufferCv.notify_all();

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
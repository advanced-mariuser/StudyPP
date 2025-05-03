#ifndef PLAYER_AUDIOPLAYER_H
#define PLAYER_AUDIOPLAYER_H

#include <portaudio.h>
#include <vector>
#include <stdexcept>
#include <iostream>

class AudioPlayer
{
private:

    PaStream* m_stream;
    std::vector<float> m_audioData;
    size_t m_position;

public:

    AudioPlayer() : m_stream(nullptr), m_position(0)
    {
        PaError err = Pa_Initialize();
        if (err != paNoError)
        {
            throw std::runtime_error("PortAudio initialization failed: " + std::string(Pa_GetErrorText(err)));
        }
    }

    ~AudioPlayer()
    {
        if (m_stream)
        {
            Pa_StopStream(m_stream);
            Pa_CloseStream(m_stream);
        }
        Pa_Terminate();
    }

    void Play(const std::vector<float>& audioData, int sampleRate)
    {
        m_audioData = audioData;
        m_position = 0;

        PaError err = Pa_OpenDefaultStream(&m_stream, 0, 1, paFloat32, sampleRate, 1024, &AudioPlayer::Callback, this);
        if (err != paNoError)
        {
            throw std::runtime_error("PortAudio stream open failed: " + std::string(Pa_GetErrorText(err)));
        }

        err = Pa_StartStream(m_stream);
        if (err != paNoError)
        {
            throw std::runtime_error("PortAudio stream start failed: " + std::string(Pa_GetErrorText(err)));
        }
    }

    static std::vector<float> GenerateSineWave(double frequency, double duration, int sampleRate)
    {
        std::vector<float> wave;
        int numSamples = static_cast<int>(duration * sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            double time = static_cast<double>(i) / sampleRate;
            wave.push_back(static_cast<float>(std::sin(2 * M_PI * frequency * time)));
        }

        return wave;
    }

    bool IsPlaying() const
    {
        return m_stream && Pa_IsStreamActive(m_stream);
    }

    void WaitUntilFinished()
    {
        while (IsPlaying())
        {
            Pa_Sleep(10);
        }
    }

private:

    static int Callback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags, void* userData)
    {
        auto* player = static_cast<AudioPlayer*>(userData);
        auto* out = static_cast<float*>(outputBuffer);

        for (unsigned long i = 0; i < framesPerBuffer; ++i)
        {
            if (player->m_position < player->m_audioData.size())
            {
                out[i] = player->m_audioData[player->m_position++];
            } else
            {
                out[i] = 0.0f;
            }
        }

        return paContinue;
    }
};

#endif //PLAYER_AUDIOPLAYER_H

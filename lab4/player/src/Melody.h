#ifndef PLAYER_MELODY_H
#define PLAYER_MELODY_H

#include "Note.h"
#include "AudioPlayer.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class Melody
{
private:
    int m_tempo{};
    vector<Note> m_notes;
    AudioPlayer m_audioPlayer;
    vector<float> m_currentWave;

public:
    explicit Melody(const string& filePath)
    {
        LoadFromFile(filePath);
    }

    void Play()
    {
        const int sampleRate = 44100;
        const double millisecondsPerNote = 60000.0 / m_tempo;
        const double secondsPerNote = millisecondsPerNote / 1000.0;

        vector<float> fullWave; // Все ноты будут здесь

        for (const auto& note: m_notes)
        {
            vector<float> wave;

            if (note.IsEmpty())
            {
                continue;
            } else if (note.IsReleased())
            {
                if (!m_currentWave.empty())
                {
                    ApplyFadeOut(m_currentWave, 0.05f, sampleRate);
                    wave = m_currentWave;
                    m_currentWave.clear();
                }
            } else
            {
                double frequency = note.GetFrequency();
                wave = AudioPlayer::GenerateSineWave(frequency, secondsPerNote, sampleRate);

                if (!m_currentWave.empty())
                {
                    ApplyFadeOut(m_currentWave, 0.05f, sampleRate);
                    MixWaves(m_currentWave, wave, 0.5f);
                    m_currentWave.clear();
                }

                m_currentWave = wave;
            }

            if (!wave.empty())
            {
                NormalizeAmplitude(wave);
                fullWave.insert(fullWave.end(), wave.begin(), wave.end());
            }
        }

        if (!fullWave.empty())
        {
            m_audioPlayer.Play(fullWave, sampleRate);
            m_audioPlayer.WaitUntilFinished();
        }
    }

private:
    static void MixWaves(const vector<float>& source, vector<float>& destination, float gain)
    {
        size_t minSize = min(source.size(), destination.size());
        for (size_t i = 0; i < minSize; ++i)
        {
            destination[i] += source[i] * gain;
        }
    }

    //затухание
    static void ApplyFadeOut(vector<float>& wave, float fadeDurationInSeconds, int sampleRate)
    {
        auto fadeSamples = static_cast<size_t>(fadeDurationInSeconds * sampleRate);
        fadeSamples = min(fadeSamples, wave.size());

        for (size_t i = 0; i < fadeSamples; ++i)
        {
            float t = static_cast<float>(i) / fadeSamples;
            wave[wave.size() - fadeSamples + i] *= (1.0f - t);
        }
    }

    static void NormalizeAmplitude(vector<float>& wave)
    {
        float maxAmplitude = 0.0f;
        for (const auto& sample: wave)
        {
            maxAmplitude = max(maxAmplitude, abs(sample));
        }

        if (maxAmplitude > 1.0f)
        {
            for (auto& sample: wave)
            {
                sample /= maxAmplitude;
            }
        }
    }

    void LoadFromFile(const string& filePath)
    {
        ifstream file(filePath);
        if (!file.is_open())
            throw runtime_error("Failed to open file: " + filePath);

        string line;
        getline(file, line);
        m_tempo = stoi(line);

        //доработать обработку пробелов, не склеивать ноты, генерировать сэмплы на ходу,
        while (getline(file, line))
        {
            if (line == "END") break;
            if (line.empty()) continue;

            m_notes.emplace_back(line);
        }
    }
};

#endif // PLAYER_MELODY_H
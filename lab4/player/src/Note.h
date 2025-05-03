#ifndef PLAYER_NOTE_H
#define PLAYER_NOTE_H

#include <string>
#include <regex>
#include <stdexcept>
#include <unordered_map>
#include <cmath>
#include <iostream>

using namespace std;

class Note
{
private:
    bool m_isEmpty = false;
    string m_note; // Название ноты (A, B, C, D, E, F, G)
    bool m_isSharp;     // Флаг диеза
    int m_octave;       // Октава (0-8)
    bool m_isReleased;  // Флаг отпускания ноты

public:
    explicit Note(const string& noteStr)
    {
        regex noteRegex(R"(([A-G])(#)?([0-8])?(-)?|(-))");
        smatch match;

        if (!regex_match(noteStr, match, noteRegex))
        {
            throw invalid_argument("Invalid note format: " + noteStr);
        }

        if (noteStr == "-")
        {
            m_note = "";
            m_isSharp = false;
            m_octave = 0;
            m_isReleased = true;
        }
        else if (noteStr.empty())
        {
            m_isEmpty = true;
        }
        else
        {
            m_note = match[1].str();
            m_isSharp = match[2].matched;
            if (match[3].matched)
                m_octave = stoi(match[3].str());
            else
                throw invalid_argument("Octave is missing in note: " + noteStr);
            m_isReleased = match[4].matched;
        }

        if (m_octave < 0 || m_octave > 8)
        {
            throw invalid_argument("Octave must be between 0 and 8: " + noteStr);
        }
    }

    [[nodiscard]] bool IsEmpty() const { return m_isEmpty; }

    [[nodiscard]] string GetNote() const { return m_note; }

    [[nodiscard]] bool IsSharp() const { return m_isSharp; }

    [[nodiscard]] int GetOctave() const { return m_octave; }

    [[nodiscard]] bool IsReleased() const { return m_isReleased; }

    [[nodiscard]] double GetFrequency() const
    {
        // Эталонная частота для ноты A4 (440 Гц)
        const double referenceFrequency = 440.0;

        static const unordered_map<string, int> noteSemitones = {
                {"C",  -9},
                {"C#", -8},
                {"D",  -7},
                {"D#", -6},
                {"E",  -5},
                {"F",  -4},
                {"F#", -3},
                {"G",  -2},
                {"G#", -1},
                {"A",  0},
                {"A#", 1},
                {"B",  2}
        };

        string noteKey = m_note + (m_isSharp ? "#" : "");

        if (noteSemitones.find(noteKey) == noteSemitones.end())
            throw invalid_argument("Invalid note: " + noteKey);

        int semitonesFromA4 = noteSemitones.at(noteKey);

        int octaveDifference = m_octave - 4; // A4 находится в 4-й октаве
        semitonesFromA4 += octaveDifference * 12; // Каждая октава — это 12 полутонов

        // Рассчитываем частоту
        return referenceFrequency * pow(2, semitonesFromA4 / 12.0);
    }
};

#endif //PLAYER_NOTE_H

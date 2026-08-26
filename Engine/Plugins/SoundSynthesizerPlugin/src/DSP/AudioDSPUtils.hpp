#pragma once

#include <cmath>
#include <cstdlib>

namespace SoundStudio
{

class AudioDSPUtils
{
public:
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 6.28318530717958647692f;

    static inline float Clamp(float val, float minVal, float maxVal)
    {
        return val < minVal ? minVal : (val > maxVal ? maxVal : val);
    }

    static inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    static inline float WhiteNoise() { return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; }

    static inline float MidiToFrequency(int midiNote) { return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f); }

    static inline float DbToLinear(float db) { return std::pow(10.0f, db * 0.05f); }

    static inline float LinearToDb(float linear) { return linear > 0.00001f ? 20.0f * std::log10(linear) : -100.0f; }
};

} // namespace SoundStudio

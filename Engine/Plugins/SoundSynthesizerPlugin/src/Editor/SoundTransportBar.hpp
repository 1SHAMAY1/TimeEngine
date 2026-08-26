#pragma once

#include <functional>

namespace SoundStudio
{

class SoundTransportBar
{
public:
    SoundTransportBar() = default;

    std::function<void()> OnPlay;
    std::function<void()> OnStop;
    std::function<void()> OnBake;

    void Draw(bool isPlaying, float &masterGain);
};

} // namespace SoundStudio

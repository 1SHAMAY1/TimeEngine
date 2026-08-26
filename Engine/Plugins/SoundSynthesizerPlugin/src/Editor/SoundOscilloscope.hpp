#pragma once

#include "../SoundStudioTypes.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <vector>

namespace SoundStudio
{

class SoundOscilloscope
{
public:
    SoundOscilloscope(size_t historySize = 512);

    void PushBlock(const AudioBufferBlock &block);
    void Draw(float width, float height);

private:
    TEArray<float> m_WaveformHistory;
    size_t m_HistoryIndex = 0;
};

} // namespace SoundStudio

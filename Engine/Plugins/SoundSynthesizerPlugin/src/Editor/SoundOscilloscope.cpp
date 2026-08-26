#include "SoundOscilloscope.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include <algorithm>

namespace SoundStudio
{

SoundOscilloscope::SoundOscilloscope(size_t historySize) { m_WaveformHistory.resize(historySize, 0.0f); }

void SoundOscilloscope::PushBlock(const AudioBufferBlock &block)
{
    if (m_WaveformHistory.empty())
        return;

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        m_WaveformHistory[m_HistoryIndex] = block.Samples[i];
        m_HistoryIndex = (m_HistoryIndex + 1) % m_WaveformHistory.size();
    }
}

void SoundOscilloscope::Draw(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f)
        return;

    TEVector2 pos = TimeGUI::GetCursorScreenPos();
    TimeGUI::Dummy(TEVector2(width, height));

    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();

    // Background & border
    drawList.AddRectFilled(pos, TEVector2(pos.x + width, pos.y + height), TIMEGUI_COL32(20, 24, 30, 255), 4.0f);
    drawList.AddRect(pos, TEVector2(pos.x + width, pos.y + height), TIMEGUI_COL32(50, 55, 65, 255), 4.0f);

    // Center baseline
    float midY = pos.y + height * 0.5f;
    drawList.AddLine(TEVector2(pos.x, midY), TEVector2(pos.x + width, midY), TIMEGUI_COL32(60, 70, 80, 255), 1.0f);

    size_t count = m_WaveformHistory.size();
    if (count >= 2)
    {
        TEVector2 prevPt(0.0f, 0.0f);
        for (size_t i = 0; i < count; ++i)
        {
            size_t sampleIdx = (m_HistoryIndex + i) % count;
            float val = std::clamp(m_WaveformHistory[sampleIdx], -1.0f, 1.0f);
            float x = pos.x + ((float)i / (float)(count - 1)) * width;
            float y = midY - val * (height * 0.5f * 0.9f);
            TEVector2 currPt(x, y);

            if (i > 0)
            {
                drawList.AddLine(prevPt, currPt, TIMEGUI_COL32(0, 255, 180, 255), 1.5f);
            }
            prevPt = currPt;
        }
    }

    // Oscilloscope Title
    drawList.AddText(TEVector2(pos.x + 8.0f, pos.y + 4.0f), TIMEGUI_COL32(180, 180, 180, 255),
                     "Live Waveform Oscilloscope");
}

} // namespace SoundStudio

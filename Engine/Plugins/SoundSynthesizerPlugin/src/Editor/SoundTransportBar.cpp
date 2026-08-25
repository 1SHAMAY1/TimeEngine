#include "SoundTransportBar.hpp"
#include "Utils/TimeGUI.hpp"

namespace SoundStudio {

void SoundTransportBar::Draw(bool isPlaying, float &masterGain)
{
    if (TimeGUI::Button(isPlaying ? "Stop (Space)" : "Play (Space)", TEVector2(110.0f, 28.0f)))
    {
        if (isPlaying && OnStop)
            OnStop();
        else if (!isPlaying && OnPlay)
            OnPlay();
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Bake Audio to WAV", TEVector2(140.0f, 28.0f)))
    {
        if (OnBake)
            OnBake();
    }

    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Master Gain", &masterGain, 0.0f, 2.0f, "%.2f");

    TimeGUI::Separator();
}

} // namespace SoundStudio

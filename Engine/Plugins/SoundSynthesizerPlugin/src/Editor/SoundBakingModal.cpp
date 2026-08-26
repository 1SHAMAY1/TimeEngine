#include "SoundBakingModal.hpp"
#include "Utils/TimeGUI.hpp"
#include <cstring>

namespace SoundStudio
{

SoundBakingModal::SoundBakingModal() {}

void SoundBakingModal::Open(TERef<SoundGraph> graph)
{
    m_TargetGraph = graph;
    m_IsOpen = true;
}

void SoundBakingModal::Draw()
{
    if (!m_IsOpen)
        return;

    TimeGUI::OpenPopup("Bake Procedural Audio to WAV");

    if (TimeGUI::BeginPopupModal("Bake Procedural Audio to WAV", &m_IsOpen, TimeGUIWindowFlags_AlwaysAutoResize))
    {
        TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Offline High-Fidelity Audio Rasterizer");
        TimeGUI::Separator();

        TimeGUI::InputText("Output Path", m_PathBuffer, sizeof(m_PathBuffer));
        TimeGUI::SliderFloat("Duration (Seconds)", &m_Options.DurationSeconds, 0.1f, 30.0f, "%.2fs");
        TimeGUI::SliderFloat("Trigger Hold Time", &m_Options.TriggerDurationSeconds, 0.05f, m_Options.DurationSeconds,
                             "%.2fs");
        TimeGUI::SliderInt("Trigger MIDI Note", &m_Options.TriggerMidiNote, 24, 108);

        TimeGUI::Spacing();
        TimeGUI::Separator();

        if (TimeGUI::Button("Start Bake", TEVector2(120.0f, 32.0f)))
        {
            m_Options.OutputPath = TEString(m_PathBuffer);
            SoundBaker::BakeGraphToWAV(m_TargetGraph, m_Options);
            m_IsOpen = false;
        }

        TimeGUI::SameLine();
        if (TimeGUI::Button("Cancel", TEVector2(100.0f, 32.0f)))
        {
            m_IsOpen = false;
        }

        TimeGUI::EndPopup();
    }
}

} // namespace SoundStudio

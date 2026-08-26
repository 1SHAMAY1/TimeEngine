#include "Editor/Settings/ProjectAudioSettings.hpp"
#include "Core/PreRequisites.h"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectAudioSettings);

void ProjectAudioSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &settings = GeneralEngineSettings::Get();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Master Output");
    TimeGUI::Separator();

    bool audioEnabled = settings.IsAudioEnabled();
    if (TimeGUI::Checkbox("Enable Audio Engine", &audioEnabled))
    {
        settings.SetAudioEnabled(audioEnabled);
    }

    float volume = settings.GetAudioVolume();
    if (TimeGUI::SliderFloat("Master Volume", &volume, 0.0f, 1.0f, "%.2f"))
    {
        settings.SetAudioVolume(volume);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Hardware & Quality");
    TimeGUI::Separator();

    int sampleRate = (int)settings.GetAudioSampleRate();
    if (TimeGUI::SliderInt("Sample Rate", &sampleRate, 8000, 96000, "%d Hz"))
    {
        settings.SetAudioSampleRate((uint32_t)sampleRate);
    }

    int channels = (int)settings.GetAudioChannels();
    if (TimeGUI::SliderInt("Channels", &channels, 1, 8, "%d"))
    {
        settings.SetAudioChannels((uint32_t)channels);
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Audio Defaults", 180.0f, 26.0f))
    {
        settings.SetAudioEnabled(true);
        settings.SetAudioVolume(1.0f);
        settings.SetAudioSampleRate(44100);
        settings.SetAudioChannels(2);
    }
}

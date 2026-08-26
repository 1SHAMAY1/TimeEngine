#include "Editor/Panels/AudioAssetEditor.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

AudioAssetEditor::AudioAssetEditor() {}

TEString AudioAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "{\n"
           "  \"Name\": \"" +
           name +
           "\",\n"
           "  \"AssetType\": \"AudioClip\",\n"
           "  \"Volume\": 1.0,\n"
           "  \"Pitch\": 1.0,\n"
           "  \"Loop\": false,\n"
           "  \"Spatial\": false\n"
           "}";
}

void AudioAssetEditor::DrawEditor(EditorTab &tab)
{
    // 1. Audio Clip Info & Transport Bar
    TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Audio Clip: %s", tab.AssetPath.c_str());
    TimeGUI::Separator();

    // Playback controls
    if (TimeGUI::Button("Play Preview", TEVector2(120.0f, 32.0f)))
    {
        AudioClipHandle handle = AudioEngine::LoadClip(tab.AssetPath);
        if (handle != InvalidAudioClip)
        {
            AudioPlayParams params;
            params.Volume = m_ClipVolume;
            params.Pitch = m_ClipPitch;
            params.Loop = m_ClipLoop;
            params.Spatial = false;
            AudioEngine::Play(handle, params);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Stop All##AudioStop", TEVector2(100.0f, 32.0f)))
    {
        AudioEngine::StopAll();
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEVector4(0.9f, 0.7f, 0.3f, 1.0f), "Clip Parameters");
    TimeGUI::SliderFloat("Volume##ClipVol", &m_ClipVolume, 0.0f, 2.0f, "%.2f");
    TimeGUI::SliderFloat("Pitch##ClipPitch", &m_ClipPitch, 0.2f, 3.0f, "%.2f");
    TimeGUI::Checkbox("Loop##ClipLoop", &m_ClipLoop);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 2. Audio Master Volumes
    TimeGUI::TextColored(TEVector4(0.3f, 0.8f, 0.4f, 1.0f), "Audio Mixer & Master Volumes");
    TimeGUI::Separator();

    float masterVol = AudioEngine::GetMasterVolume();
    if (TimeGUI::SliderFloat("Master Volume", &masterVol, 0.0f, 2.0f, "%.2f"))
        AudioEngine::SetMasterVolume(masterVol);

    float musicVol = AudioEngine::GetMusicVolume();
    if (TimeGUI::SliderFloat("Music Volume", &musicVol, 0.0f, 2.0f, "%.2f"))
        AudioEngine::SetMusicVolume(musicVol);

    float sfxVol = AudioEngine::GetSFXVolume();
    if (TimeGUI::SliderFloat("SFX Volume", &sfxVol, 0.0f, 2.0f, "%.2f"))
        AudioEngine::SetSFXVolume(sfxVol);

    float voiceVol = AudioEngine::GetVoiceVolume();
    if (TimeGUI::SliderFloat("Voice Volume", &voiceVol, 0.0f, 2.0f, "%.2f"))
        AudioEngine::SetVoiceVolume(voiceVol);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 3. Hardware Devices
    TimeGUI::TextColored(TEVector4(0.4f, 0.7f, 0.95f, 1.0f), "Audio Hardware Devices");
    auto devices = AudioEngine::GetOutputDevices();
    if (!devices.empty())
    {
        TEArray<const char *> deviceCstrs;
        for (const auto &d : devices)
            deviceCstrs.push_back(d.c_str());

        if (m_SelectedDeviceIndex < 0 || m_SelectedDeviceIndex >= (int)devices.size())
            m_SelectedDeviceIndex = 0;

        if (TimeGUI::Combo("Output Device", &m_SelectedDeviceIndex, deviceCstrs.data(), (int)deviceCstrs.size()))
            AudioEngine::SetOutputDevice(devices[m_SelectedDeviceIndex]);
    }

    TimeGUI::Spacing();
    TimeGUI::TextDisabled("Active Sounds: %zu | Loaded Clips: %zu | Dilation: %.2fx",
                          AudioEngine::GetActiveSoundCount(), AudioEngine::GetLoadedClipCount(),
                          AudioEngine::GetTimeDilationFactor());
}

void AudioAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background in Ocean Blue
    dl.AddRectFilled(min, max, IM_COL32(25, 95, 160, 230), 4.0f);

    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(15, 28, 45, 255), 2.0f);

    // Speaker cone + sound wave rings
    float cx = (cMin.x + cMax.x) * 0.5f - 4.0f;
    float cy = (cMin.y + cMax.y) * 0.5f;

    // Speaker body
    dl.AddRectFilled(TEVector2(cx - 8.0f, cy - 6.0f), TEVector2(cx - 2.0f, cy + 6.0f), IM_COL32(0, 210, 255, 255),
                     1.0f);
    dl.AddTriangleFilled(TEVector2(cx - 2.0f, cy - 6.0f), TEVector2(cx + 4.0f, cy - 11.0f),
                         TEVector2(cx + 4.0f, cy + 11.0f), IM_COL32(0, 210, 255, 255));

    // Sound waves
    dl.AddCircle(TEVector2(cx + 4.0f, cy), 8.0f, IM_COL32(100, 230, 255, 200), 16, 2.0f);
    dl.AddCircle(TEVector2(cx + 4.0f, cy), 14.0f, IM_COL32(100, 230, 255, 120), 16, 1.5f);
}

TE_REGISTER_ASSET_EDITOR(AudioAssetEditor);

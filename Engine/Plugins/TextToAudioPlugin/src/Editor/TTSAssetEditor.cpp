#include "TTSAssetEditor.hpp"
#include "../TTSAudioEngine.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"
#include "Utils/TEFileSystem.hpp"

TTSAssetEditor::TTSAssetEditor()
{
}

TEString TTSAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "{\n"
           "  \"Text\": \"<color=gold>Hello!</color> <pause time=\\\"300ms\\\"/> Welcome to TimeEngine speech synthesis.\",\n"
           "  \"Volume\": 1.0,\n"
           "  \"Pitch\": 1.0,\n"
           "  \"Rate\": 1.0,\n"
           "  \"EnableAcoustics\": true\n"
           "}";
}

void TTSAssetEditor::DrawEditor(EditorTab &tab)
{
    static TEString s_LastPath = "";
    if (s_LastPath != tab.AssetPath)
    {
        s_LastPath = tab.AssetPath;
        if (TEFileSystem::Exists(tab.AssetPath))
        {
            TEString content = TEFileSystem::ReadAllText(tab.AssetPath);
            size_t textPos = content.find("\"Text\": \"");
            if (textPos != TEString::npos)
            {
                size_t start = textPos + 9;
                size_t end = content.find("\"", start);
                if (end != TEString::npos)
                    m_InputText = content.substr(start, end - start);
            }
        }
    }

    TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Local Offline Text-to-Audio / Speech Synthesis");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset##TTSSave"))
    {
        TEString json = "{\n  \"Text\": \"" + m_InputText + "\",\n  \"Volume\": " +
                        TEString::FromFloat(m_Volume) + ",\n  \"Pitch\": " +
                        TEString::FromFloat(m_Pitch) + ",\n  \"Rate\": " +
                        TEString::FromFloat(m_Rate) + ",\n  \"EnableAcoustics\": " +
                        (m_EnableAcoustics ? "true" : "false") + "\n}";
        if (TEFileSystem::WriteAllText(tab.AssetPath, json))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::Separator();

    // 1. Text Input Area
    TimeGUI::Text("Enter Text or Acoustic Markup:");
    if (TimeGUI::InputTextMultiline("##TTSInput", m_InputText, TEVector2(-1.0f, 100.0f)))
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Spacing();
    if (TimeGUI::Checkbox("Enable Acoustic RichText Tags (<pause>, <pitch>, <rate>, etc.)", &m_EnableAcoustics))
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 2. Prosody Sliders
    TimeGUI::TextColored(TEVector4(0.9f, 0.7f, 0.3f, 1.0f), "Vocal Prosody & Speed");
    if (TimeGUI::SliderFloat("Volume", &m_Volume, 0.0f, 1.0f, "%.2f"))
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    if (TimeGUI::SliderFloat("Pitch Modifier", &m_Pitch, 0.5f, 2.0f, "%.2f"))
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    if (TimeGUI::SliderFloat("Speech Rate", &m_Rate, 0.5f, 3.0f, "%.2f"))
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 3. Voice Selection
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 0.5f, 1.0f), "Installed System Voices");
    auto voices = TTSAudioEngine::Get().GetAvailableVoices();
    if (!voices.empty())
    {
        TEArray<const char *> voiceCstrs;
        for (const auto &v : voices)
            voiceCstrs.push_back(v.c_str());

        if (m_SelectedVoiceIndex < 0 || m_SelectedVoiceIndex >= (int)voices.size())
            m_SelectedVoiceIndex = 0;

        if (TimeGUI::Combo("Active Voice", &m_SelectedVoiceIndex, voiceCstrs.data(), (int)voiceCstrs.size()))
        {
            TTSAudioEngine::Get().SetVoice(voices[m_SelectedVoiceIndex]);
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 4. Playback Buttons
    if (TimeGUI::Button("Speak (Async)", TEVector2(130.0f, 32.0f)))
    {
        TTSSpeakRequest req;
        req.Text = TEString(m_InputText);
        req.Volume = m_Volume;
        req.Pitch = m_Pitch;
        req.Rate = m_Rate;
        req.EnableRichTextAcoustics = m_EnableAcoustics;
        req.Mode = TTSSpeakMode::Async;
        TTSAudioEngine::Get().Speak(req);
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Speak (Sync)", TEVector2(130.0f, 32.0f)))
    {
        TTSSpeakRequest req;
        req.Text = TEString(m_InputText);
        req.Volume = m_Volume;
        req.Pitch = m_Pitch;
        req.Rate = m_Rate;
        req.EnableRichTextAcoustics = m_EnableAcoustics;
        req.Mode = TTSSpeakMode::Sync;
        TTSAudioEngine::Get().Speak(req);
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Stop All", TEVector2(100.0f, 32.0f)))
    {
        TTSAudioEngine::Get().StopAll();
    }

    TimeGUI::Spacing();
    TimeGUI::TextDisabled("Status: %s | Queue Size: %zu",
                          TTSAudioEngine::Get().IsSpeaking() ? "Speaking..." : "Idle",
                          TTSAudioEngine::Get().GetQueueSize());
}

void TTSAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background in Emerald/Teal
    dl.AddRectFilled(min, max, IM_COL32(35, 120, 100, 230), 4.0f);

    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(18, 30, 28, 255), 2.0f);

    // Speech bubble outline + sound waves
    float cx = (cMin.x + cMax.x) * 0.5f;
    float cy = (cMin.y + cMax.y) * 0.5f;
    dl.AddCircle(TEVector2(cx, cy), (cMax.x - cMin.x) * 0.28f, IM_COL32(0, 220, 180, 255), 24, 2.0f);
    dl.AddLine(TEVector2(cx - 6.0f, cy), TEVector2(cx + 6.0f, cy), IM_COL32(255, 255, 255, 255), 2.0f);
    dl.AddLine(TEVector2(cx - 3.0f, cy - 4.0f), TEVector2(cx + 3.0f, cy - 4.0f), IM_COL32(255, 255, 255, 255), 2.0f);
}

TE_REGISTER_ASSET_EDITOR(TTSAssetEditor);

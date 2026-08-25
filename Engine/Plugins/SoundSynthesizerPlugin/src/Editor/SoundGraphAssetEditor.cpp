#include "SoundGraphAssetEditor.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

namespace SoundStudio {

SoundGraphAssetEditor::SoundGraphAssetEditor()
{
    m_ActiveGraph = CreateRef<SoundGraph>();
    m_Evaluator = CreateScope<SoundGraphEvaluator>(m_ActiveGraph);

    // Wire virtual keyboard callbacks
    m_Keyboard.OnNoteOn = [this](int midiNote, float velocity) {
        if (m_Evaluator)
        {
            m_Evaluator->TriggerNote(midiNote, velocity);
            m_IsPlaying = true;
        }
    };

    m_Keyboard.OnNoteOff = [this]() {
        if (m_Evaluator)
            m_Evaluator->TriggerNoteOff();
    };
}

TEString SoundGraphAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "{\n"
           "  \"Name\": \"" + name + "\",\n"
           "  \"AssetType\": \"SoundGraph\",\n"
           "  \"Description\": \"Procedural Audio DSP Graph\",\n"
           "  \"Nodes\": []\n"
           "}";
}

void SoundGraphAssetEditor::SetActiveGraph(TERef<SoundGraph> graph)
{
    m_ActiveGraph = graph;
    if (m_Evaluator)
        m_Evaluator->SetGraph(m_ActiveGraph);
}

void SoundGraphAssetEditor::Play()
{
    m_IsPlaying = true;
    if (m_Evaluator)
        m_Evaluator->TriggerNote(69, 1.0f);
}

void SoundGraphAssetEditor::Stop()
{
    m_IsPlaying = false;
    if (m_Evaluator)
        m_Evaluator->Reset();
}

void SoundGraphAssetEditor::DrawEditor(EditorTab &tab)
{
    auto asset = std::dynamic_pointer_cast<SoundGraphAsset>(tab.LoadedAsset);
    if (!asset)
    {
        asset = CreateRef<SoundGraphAsset>();
        asset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = asset;
        m_ActiveGraph = asset->GetGraph();
        if (!m_ActiveGraph)
        {
            m_ActiveGraph = CreateRef<SoundGraph>();
            asset->SetGraph(m_ActiveGraph);
        }
        if (m_Evaluator)
            m_Evaluator->SetGraph(m_ActiveGraph);
    }
    else if (m_ActiveGraph != asset->GetGraph())
    {
        m_ActiveGraph = asset->GetGraph();
        if (m_Evaluator)
            m_Evaluator->SetGraph(m_ActiveGraph);
    }

    // Tick audio playback if active
    if (m_IsPlaying && m_Evaluator)
    {
        StereoAudioBlock stereo;
        m_Evaluator->ProcessBlock(stereo);
        m_Oscilloscope.PushBlock(stereo.Left);
        AudioEngine::SubmitPCMFrames(stereo.Left.Samples, AUDIO_BLOCK_SIZE, DEFAULT_SAMPLE_RATE, 1);
    }

    // 1. Top Transport Toolbar
    TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Sound Graph: %s", tab.AssetPath.c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button(m_IsPlaying ? "Stop (Space)##SynthPlay" : "Play (Space)##SynthPlay", TEVector2(110.0f, 26.0f)))
    {
        if (m_IsPlaying)
            Stop();
        else
            Play();
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Bake Audio to WAV##SynthBake", TEVector2(140.0f, 26.0f)))
    {
        m_BakingModal.Open(m_ActiveGraph);
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset##SynthSave"))
    {
        if (asset && asset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Master Gain##SynthGain", &m_MasterGain, 0.0f, 2.0f, "%.2f");

    TimeGUI::Separator();

    // 2. Main Middle Canvas Area
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    float keyboardHeight = 65.0f;
    float scopeHeight = 75.0f;
    float graphHeight = canvasSize.y - keyboardHeight - scopeHeight - 20.0f;

    m_Canvas.Draw(m_ActiveGraph, canvasSize.x, std::max(150.0f, graphHeight));

    // 3. Oscilloscope Visualizer
    m_Oscilloscope.Draw(canvasSize.x, scopeHeight);

    // 4. Virtual Piano Keyboard
    m_Keyboard.Draw(canvasSize.x, keyboardHeight);

    // Render Baking Modal if open
    m_BakingModal.Draw();
}

void SoundGraphAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background in Neon Teal / Dark Cyan
    dl.AddRectFilled(min, max, IM_COL32(18, 90, 95, 230), 4.0f);

    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(12, 25, 28, 255), 2.0f);

    // Oscilloscope sine wave line
    float cy = (cMin.y + cMax.y) * 0.5f;
    float step = (cMax.x - cMin.x) / 16.0f;
    for (int i = 0; i < 16; ++i)
    {
        float x1 = cMin.x + i * step;
        float x2 = cMin.x + (i + 1) * step;
        float y1 = cy + sinf(i * 0.8f) * 6.0f;
        float y2 = cy + sinf((i + 1) * 0.8f) * 6.0f;
        dl.AddLine(TEVector2(x1, y1), TEVector2(x2, y2), IM_COL32(0, 240, 210, 255), 2.0f);
    }
}

TE_REGISTER_ASSET_EDITOR(SoundGraphAssetEditor);

} // namespace SoundStudio


#include "SoundSynthesizerPlugin.hpp"
#include "Core/Log.h"
#include "Editor/EditorMode.hpp"
#include "Editor/SoundGraphAssetEditor.hpp"
#include "Graph/SoundNodeRegistry.hpp"
#include "MCP/SoundSynthMCPTools.hpp"

void SoundSynthesizerPlugin::OnLoad()
{
    TE_CORE_INFO("[SoundSynthesizerPlugin] Initializing SoundSynthesizerPlugin...");

    // 1. Initialize default DSP nodes
    SoundStudio::SoundNodeRegistry::InitializeDefaults();

    // 2. Register MCP Tools
    SoundStudio::SoundSynthMCPTools::RegisterTools();

    TE_CORE_INFO("[SoundSynthesizerPlugin] SoundSynthesizerPlugin loaded successfully.");
}

#include <cmath>

void SoundSynthesizerPlugin::OnUnload()
{
    TE_CORE_INFO("[SoundSynthesizerPlugin] Unloading SoundSynthesizerPlugin...");
    TE_CORE_INFO("[SoundSynthesizerPlugin] SoundSynthesizerPlugin unloaded.");
}

void SoundSynthesizerPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;

    unsigned int bgCol = 0xFF280C26;
    unsigned int borderCol = 0xFFE056FD;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Oscilloscope sine waveform
    const int waveSteps = 24;
    TEVector2 wavePts[24];
    for (int i = 0; i < waveSteps; ++i)
    {
        float t = (float)i / (float)(waveSteps - 1);
        float px = min.x + 8.0f + t * (w - 16.0f);
        float py = min.y + 18.0f + sinf(t * 6.2831853f * 1.5f) * 7.0f;
        wavePts[i] = TEVector2(px, py);
    }
    dl.AddPolyline(wavePts, waveSteps, 0xFF00FFC8, 0, 2.0f);

    // Frequency spectrum equalizer bars
    float barX = min.x + 10.0f;
    float barY = max.y - 7.0f;
    float barHeights[5] = {6.0f, 12.0f, 16.0f, 10.0f, 14.0f};
    for (int i = 0; i < 5; ++i)
    {
        dl.AddLine(TEVector2(barX + i * 6.5f, barY), TEVector2(barX + i * 6.5f, barY - barHeights[i]), 0xFFE056FD,
                   3.0f);
    }
}

TE_REGISTER_PLUGIN(SoundSynthesizerPlugin);

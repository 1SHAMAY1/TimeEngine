#include "TextToAudioPlugin.hpp"
#include "Core/Log.h"
#include "TTSAudioEngine.hpp"

void TextToAudioPlugin::OnLoad()
{
    TE_CORE_INFO("[TextToAudioPlugin] Initializing TextToAudioPlugin...");
    TTSAudioEngine::Get().Initialize();
    TE_CORE_INFO("[TextToAudioPlugin] TextToAudioPlugin loaded successfully.");
}

void TextToAudioPlugin::OnUnload()
{
    TE_CORE_INFO("[TextToAudioPlugin] Unloading TextToAudioPlugin...");
    TTSAudioEngine::Get().Shutdown();
    TE_CORE_INFO("[TextToAudioPlugin] TextToAudioPlugin unloaded.");
}

void TextToAudioPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);

    unsigned int bgCol = 0xFF0B2428;
    unsigned int borderCol = 0xFF20BF6B;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Document lines on left
    dl.AddRectFilled(TEVector2(min.x + 7.0f, min.y + 11.0f), TEVector2(min.x + 19.0f, max.y - 11.0f), 0xFF15434A, 2.0f);
    dl.AddLine(TEVector2(min.x + 9.0f, min.y + 15.0f), TEVector2(min.x + 17.0f, min.y + 15.0f), 0xFFFFFFFF, 1.2f);
    dl.AddLine(TEVector2(min.x + 9.0f, min.y + 20.0f), TEVector2(min.x + 17.0f, min.y + 20.0f), 0xFF00CEC9, 1.2f);
    dl.AddLine(TEVector2(min.x + 9.0f, min.y + 25.0f), TEVector2(min.x + 15.0f, min.y + 25.0f), 0xFFFFFFFF, 1.2f);

    // Acoustic speaker cone
    TEVector2 spPts[6] = {TEVector2(min.x + 23.0f, c.y - 4.0f), TEVector2(min.x + 26.0f, c.y - 4.0f),
                          TEVector2(min.x + 31.0f, c.y - 8.0f), TEVector2(min.x + 31.0f, c.y + 8.0f),
                          TEVector2(min.x + 26.0f, c.y + 4.0f), TEVector2(min.x + 23.0f, c.y + 4.0f)};
    dl.AddConvexPolyFilled(spPts, 6, 0xFF2ED573);

    // Soundwave ripples
    dl.AddCircle(TEVector2(min.x + 28.0f, c.y), 6.0f, 0xFF00D2D3, 16, 1.2f);
    dl.AddCircle(TEVector2(min.x + 28.0f, c.y), 10.5f, 0x9955EFC4, 16, 1.2f);
}

TE_REGISTER_PLUGIN(TextToAudioPlugin);

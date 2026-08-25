#include "Lighting2DPlugin.hpp"
#include "Core/Log.h"


void Lighting2DPlugin::OnLoad()
{
    TE_CORE_INFO("[Lighting2DPlugin] 2D Dynamic Lighting & Ray Tracing Plugin Loaded Successfully.");
}

#include <cmath>

void Lighting2DPlugin::OnUnload()
{
    TE_CORE_INFO("[Lighting2DPlugin] Unloading 2D Dynamic Lighting & Ray Tracing Plugin...");
}

void Lighting2DPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);

    unsigned int bgCol = 0xFF1A1D2B;
    unsigned int borderCol = 0xFFF1C40F;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Radial attenuation glow disks
    dl.AddCircleFilled(c, 18.0f, 0x22F39C12);
    dl.AddCircleFilled(c, 12.0f, 0x44F1C40F);
    dl.AddCircleFilled(c, 6.0f, 0x88FFEAA7);
    dl.AddCircleFilled(c, 3.0f, 0xFFFFFFFF);

    // Radiating light rays
    const float rayDistMin = 9.0f;
    const float rayDistMax = 16.0f;
    for (int i = 0; i < 8; ++i)
    {
        float ang = i * (3.14159265f / 4.0f);
        float cosA = cosf(ang);
        float sinA = sinf(ang);
        TEVector2 r1(c.x + cosA * rayDistMin, c.y + sinA * rayDistMin);
        TEVector2 r2(c.x + cosA * rayDistMax, c.y + sinA * rayDistMax);
        dl.AddLine(r1, r2, 0xCCF9CA24, 1.5f);
    }
}

TE_REGISTER_PLUGIN(Lighting2DPlugin);


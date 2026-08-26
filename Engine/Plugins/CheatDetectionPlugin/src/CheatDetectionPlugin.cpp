#include "CheatDetectionPlugin.hpp"

TE_REGISTER_PLUGIN(CheatDetectionPlugin)

void CheatDetectionPlugin::OnLoad()
{
    // TODO: Register network kinematic validators once networking layer is ready
}

void CheatDetectionPlugin::OnUnload()
{
    // TODO: Cleanup validator registrations
}

void CheatDetectionPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF2B0909, 6.0f);
    dl.AddRect(min, max, 0xFF6B1414, 6.0f, 0, 1.0f);

    // Shield icon
    dl.AddCircle(c, 9.0f, 0xFFE63946, 16, 2.0f);
    dl.AddLine(TEVector2(c.x - 5.0f, c.y), TEVector2(c.x, c.y + 5.0f), 0xFFF1FAEE, 2.0f);
    dl.AddLine(TEVector2(c.x, c.y + 5.0f), TEVector2(c.x + 6.0f, c.y - 4.0f), 0xFFF1FAEE, 2.0f);
}

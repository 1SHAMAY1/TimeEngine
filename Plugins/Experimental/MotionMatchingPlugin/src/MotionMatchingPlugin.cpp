#include "MotionMatchingPlugin.hpp"

TE_REGISTER_PLUGIN(MotionMatchingPlugin)

void MotionMatchingPlugin::OnLoad()
{
    // TODO: Register MotionMatchingComponent
}

void MotionMatchingPlugin::OnUnload()
{
    // TODO: Unregister component
}

void MotionMatchingPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF1B263B, 6.0f);
    dl.AddRect(min, max, 0xFF415A77, 6.0f, 0, 1.0f);

    // Trajectory arc
    dl.AddCircleFilled(TEVector2(c.x - 10.0f, c.y + 6.0f), 3.0f, 0xFF778DA9);
    dl.AddCircleFilled(TEVector2(c.x, c.y - 2.0f), 4.0f, 0xFFE0E1DD);
    dl.AddCircleFilled(TEVector2(c.x + 12.0f, c.y - 8.0f), 3.0f, 0xFF54A0FF);
    dl.AddLine(TEVector2(c.x - 10.0f, c.y + 6.0f), TEVector2(c.x, c.y - 2.0f), 0xFF54A0FF, 2.0f);
    dl.AddLine(TEVector2(c.x, c.y - 2.0f), TEVector2(c.x + 12.0f, c.y - 8.0f), 0xFF54A0FF, 2.0f);
}

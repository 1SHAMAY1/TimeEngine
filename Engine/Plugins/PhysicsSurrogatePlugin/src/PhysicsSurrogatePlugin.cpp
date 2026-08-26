#include "PhysicsSurrogatePlugin.hpp"

TE_REGISTER_PLUGIN(PhysicsSurrogatePlugin)

void PhysicsSurrogatePlugin::OnLoad()
{
    // TODO: Register PhysicsSurrogateComponent
}

void PhysicsSurrogatePlugin::OnUnload()
{
    // TODO: Cleanup component
}

void PhysicsSurrogatePlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF14213D, 6.0f);
    dl.AddRect(min, max, 0xFFFCA311, 6.0f, 0, 1.0f);

    // Wave motion / Cape flow
    dl.AddCircleFilled(TEVector2(c.x - 10.0f, c.y - 6.0f), 3.0f, 0xFFE5E5E5);
    dl.AddCircleFilled(TEVector2(c.x, c.y + 4.0f), 3.0f, 0xFFFCA311);
    dl.AddCircleFilled(TEVector2(c.x + 10.0f, c.y - 2.0f), 3.0f, 0xFFE5E5E5);
    dl.AddLine(TEVector2(c.x - 10.0f, c.y - 6.0f), TEVector2(c.x, c.y + 4.0f), 0xFFFCA311, 1.5f);
    dl.AddLine(TEVector2(c.x, c.y + 4.0f), TEVector2(c.x + 10.0f, c.y - 2.0f), 0xFFFCA311, 1.5f);
}

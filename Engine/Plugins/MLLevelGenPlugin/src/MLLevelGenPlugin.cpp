#include "MLLevelGenPlugin.hpp"

TE_REGISTER_PLUGIN(MLLevelGenPlugin)

void MLLevelGenPlugin::OnLoad()
{
    // TODO: Register PCGMLWFCNode and PCGGANRoomNode into PCGPlugin Node Palette
}

void MLLevelGenPlugin::OnUnload()
{
    // TODO: Cleanup node registrations
}

void MLLevelGenPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF1E2818, 6.0f);
    dl.AddRect(min, max, 0xFF3E5A28, 6.0f, 0, 1.0f);
    
    // Dungeon grid tiles
    dl.AddRectFilled(TEVector2(c.x - 10.0f, c.y - 10.0f), TEVector2(c.x - 2.0f, c.y - 2.0f), 0xFF55A630);
    dl.AddRectFilled(TEVector2(c.x + 2.0f, c.y - 10.0f), TEVector2(c.x + 10.0f, c.y - 2.0f), 0xFF80B918);
    dl.AddRectFilled(TEVector2(c.x - 10.0f, c.y + 2.0f), TEVector2(c.x - 2.0f, c.y + 10.0f), 0xFF2B9348);
    dl.AddRectFilled(TEVector2(c.x + 2.0f, c.y + 2.0f), TEVector2(c.x + 10.0f, c.y + 10.0f), 0xFFAACC00);
}

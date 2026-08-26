#include "LLMDialoguePlugin.hpp"

TE_REGISTER_PLUGIN(LLMDialoguePlugin)

void LLMDialoguePlugin::OnLoad()
{
    // TODO: Register LLMDialogueTask into StateTree and DialogueRunner nodes
}

void LLMDialoguePlugin::OnUnload()
{
    // TODO: Cleanup client connections
}

void LLMDialoguePlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF1D2D44, 6.0f);
    dl.AddRect(min, max, 0xFF748CAB, 6.0f, 0, 1.0f);

    // Chat bubble
    dl.AddRectFilled(TEVector2(c.x - 10.0f, c.y - 8.0f), TEVector2(c.x + 10.0f, c.y + 4.0f), 0xFFF0EBD8, 3.0f);
    dl.AddCircleFilled(TEVector2(c.x - 5.0f, c.y - 2.0f), 1.5f, 0xFF1D2D44);
    dl.AddCircleFilled(TEVector2(c.x, c.y - 2.0f), 1.5f, 0xFF1D2D44);
    dl.AddCircleFilled(TEVector2(c.x + 5.0f, c.y - 2.0f), 1.5f, 0xFF1D2D44);
}

#include "TextureUpscalerPlugin.hpp"

TE_REGISTER_PLUGIN(TextureUpscalerPlugin)

void TextureUpscalerPlugin::OnLoad()
{
    // TODO: Register Editor Upscale Action into AssetInspector / SpriteEditor
}

void TextureUpscalerPlugin::OnUnload()
{
    // TODO: Cleanup actions
}

void TextureUpscalerPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF2A1B3D, 6.0f);
    dl.AddRect(min, max, 0xFF824C71, 6.0f, 0, 1.0f);

    // Pixel to smooth sparkle
    dl.AddRectFilled(TEVector2(c.x - 10.0f, c.y - 4.0f), TEVector2(c.x - 4.0f, c.y + 4.0f), 0xFFD76F86);
    dl.AddLine(TEVector2(c.x + 2.0f, c.y), TEVector2(c.x + 10.0f, c.y), 0xFFFCA311, 2.0f);
    dl.AddLine(TEVector2(c.x + 6.0f, c.y - 4.0f), TEVector2(c.x + 6.0f, c.y + 4.0f), 0xFFFCA311, 2.0f);
}

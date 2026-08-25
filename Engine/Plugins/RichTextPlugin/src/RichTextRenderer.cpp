#include "RichTextRenderer.hpp"
#include "Utils/TimeGUI.hpp"


void RichTextRenderer::DrawToImGui(const RichTextLayoutResult &layout,
                                   const TERef<FontAsset> &font,
                                   const TEVector2 &screenPos,
                                   const std::function<void(const TEString &)> &onLinkClicked)
{
    if (!font || layout.GlyphQuads.IsEmpty())
        return;

    auto atlasTex = font->GetAtlasTexture();
    if (!atlasTex || atlasTex->GetRendererID() == 0)
        return;

    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    TimeGUITextureID texId = (TimeGUITextureID)(intptr_t)atlasTex->GetRendererID();

    TEVector2 mousePos = TimeGUI::GetMousePos();
    TEVector2 localMousePos(mousePos.x - screenPos.x, mousePos.y - screenPos.y);
    TEString hoveredLink = RichTextLayoutEngine::HitTestLink(layout, localMousePos);

    if (!hoveredLink.IsEmpty())
    {
        TimeGUI::SetMouseCursor(TimeGUIMouseCursor_Hand);
        if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left) && onLinkClicked)
        {
            onLinkClicked(hoveredLink);
        }
    }

    for (const auto &g : layout.GlyphQuads)
    {
        // Skip invisible / clipped characters
        if (g.Color.GetValue().a <= 0.0f)
            continue;

        float x0 = screenPos.x + g.Position.x;
        float y0 = screenPos.y + g.Position.y;
        float x1 = x0 + g.Size.x;
        float y1 = y0 + g.Size.y;

        auto col = g.Color.GetValue();
        unsigned int glyphCol = TIMEGUI_COL32(static_cast<int>(col.r * 255), static_cast<int>(col.g * 255),
                                              static_cast<int>(col.b * 255), static_cast<int>(col.a * 255));

        // Draw glyph quad
        drawList.AddImage(texId, TEVector2(x0, y0), TEVector2(x1, y1),
                          TEVector2(g.UV.x, g.UV.y), TEVector2(g.UV.z, g.UV.w), glyphCol);

        // Draw underline if enabled
        if (g.Style.Underline)
        {
            float lineY = y1 + 1.0f;
            drawList.AddLine(TEVector2(x0, lineY), TEVector2(x1, lineY), glyphCol, 1.5f);
        }

        // Draw strikethrough if enabled
        if (g.Style.Strikethrough)
        {
            float strikeY = y0 + (y1 - y0) * 0.5f;
            drawList.AddLine(TEVector2(x0, strikeY), TEVector2(x1, strikeY), glyphCol, 1.5f);
        }
    }
}


#pragma once

#include "Core/Asset/FontAsset.hpp"
#include "RichTextLayout.hpp"
#include "RichTextTypes.hpp"
#include <functional>


class RichTextRenderer
{
public:
    RichTextRenderer() = default;

    /// Renders animated rich text directly into an ImGui/TimeGUI canvas with mouse hover/click callbacks
    static void DrawToImGui(const RichTextLayoutResult &layout,
                            const TERef<FontAsset> &font,
                            const TEVector2 &screenPos,
                            const std::function<void(const TEString &)> &onLinkClicked = nullptr);
};


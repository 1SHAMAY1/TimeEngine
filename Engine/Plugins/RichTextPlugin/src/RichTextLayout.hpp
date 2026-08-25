#pragma once

#include "Core/Asset/FontAsset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "RichTextParser.hpp"
#include "RichTextTypes.hpp"
#include "Utils/MathUtils.hpp"
#include <vector>


struct RichTextLayoutResult
{
    TEArray<RichTextGlyphQuad> GlyphQuads;
    TEArray<RichTextLinkHitbox> LinkHitboxes;
    TEVector2 Bounds{0.0f, 0.0f};
    size_t TotalCharacterCount = 0;
};

class RichTextLayoutEngine
{
public:
    RichTextLayoutEngine() = default;

    /// Computes layout geometry, wrapping, alignment, and hitboxes for parsed rich text spans
    static RichTextLayoutResult ComputeLayout(const TEArray<RichTextSpan> &spans,
                                             const TERef<FontAsset> &defaultFont,
                                             float wrapWidth = 0.0f,
                                             float defaultLineSpacing = 1.2f);

    /// Tests if a local mouse position hovers over an interactive hyperlink hitbox
    static TEString HitTestLink(const RichTextLayoutResult &layout, const TEVector2 &localPos);
};


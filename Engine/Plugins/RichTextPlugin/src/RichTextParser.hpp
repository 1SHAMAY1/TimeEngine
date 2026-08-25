#pragma once

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "RichTextTable.hpp"
#include "RichTextTypes.hpp"
#include "Utils/TEString.hpp"
#include <stack>


struct RichTextSpan
{
    TEString Text;
    RichTextStyle Style;
    ERichTextAlign Alignment = ERichTextAlign::Left;
    bool IsIcon = false;
};

class RichTextParser
{
public:
    RichTextParser() = default;

    /// Parses a TEString markup string into an array of styled spans
    static TEArray<RichTextSpan> Parse(const TEString &markupText,
                                       const TERef<RichTextTable> &styleTable = nullptr,
                                       const RichTextStyle &defaultStyle = RichTextStyle{},
                                       const TEMap<TEString, TEString> &variables = {});

    /// Strips all markup tags and returns raw plain text
    static TEString StripTags(const TEString &markupText);

    /// Parses a color hex string (#RRGGBB, #RRGGBBAA, or named color)
    static TEColor ParseColor(const TEString &colorStr);
};


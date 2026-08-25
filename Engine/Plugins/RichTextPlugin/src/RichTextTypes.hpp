#pragma once

#include "Core/Asset/Asset.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <cstdint>


enum class ERichTextAlign
{
    Left = 0,
    Center,
    Right,
    Justify
};

enum class ERichTextEffect
{
    None = 0,
    Wave,
    Shake,
    Rainbow,
    Typewriter,
    Fade
};

struct RichTextStyle
{
    TEString StyleName = "Default";
    AssetHandle FontAsset = 0;
    float FontSize = 16.0f;
    float Scale = 1.0f;
    TEColor TextColor{1.0f, 1.0f, 1.0f, 1.0f};
    TEColor BackgroundColor{0.0f, 0.0f, 0.0f, 0.0f};

    bool IsGradient = false;
    TEColor GradientTo{1.0f, 1.0f, 1.0f, 1.0f};
    int GradientDirection = 0; // 0 = Horizontal, 1 = Vertical

    bool Bold = false;
    bool Italic = false;
    bool Underline = false;
    bool Strikethrough = false;

    ERichTextEffect Effect = ERichTextEffect::None;
    TEVector4 EffectParams{2.0f, 2.0f, 2.0f, 1.0f}; // Speed, Amplitude/Intensity, Frequency, etc.

    AssetHandle IconTexture = 0;
    TEString SpriteName;
    TEString LinkID;
};

struct RichTextGlyphQuad
{
    TEString Codepoint;
    TEVector2 Position{0.0f, 0.0f};
    TEVector2 Size{0.0f, 0.0f};
    TEVector4 UV{0.0f, 0.0f, 1.0f, 1.0f};
    TEColor Color{1.0f, 1.0f, 1.0f, 1.0f};
    RichTextStyle Style;
    size_t CharacterIndex = 0;
    bool IsIcon = false;
};

struct RichTextLinkHitbox
{
    TEString LinkID;
    TEVector2 Min{0.0f, 0.0f};
    TEVector2 Max{0.0f, 0.0f};
};


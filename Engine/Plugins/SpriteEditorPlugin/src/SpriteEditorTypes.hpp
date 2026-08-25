#pragma once

#include "Core/Asset/AssetManager.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "SpriteModeLibrary.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>



enum class SpriteCreationMode
{
    Code,
    Vector,
    PixelPaint
};

enum class VectorShapeType
{
    Selection,
    Pen,
    Rectangle,
    Circle,
    Triangle,
    Semicircle
};

struct VectorElement
{
    VectorShapeType Type;
    TEArray<TEVector2> Points;
    TEArray<TEArray<TEVector2>> SubPaths;
    float Radius = 0.0f;
    float RadiusY = 0.0f;
    TEVector4 FillColor = TEVector4(1, 1, 1, 1);
    TEVector4 StrokeColor = TEVector4(0, 0, 0, 1);
    float StrokeThickness = 1.0f;
    float StrokeRounding = 0.0f;
    float FillRounding = 0.0f;
    bool Subtract = false;
    bool Selected = false;
};

enum class KeyType
{
    Float,
    Bool,
    Color,
    Vec2
};

struct CustomKeyword
{
    TEString Name;
    KeyType Type = KeyType::Float;
    float ValFloat = 0.0f;
    bool ValBool = false;
    float ValColor[4] = {1, 1, 1, 1};
    float ValVec2[2] = {0, 0};
};

struct PixelLayer
{
    TEString Name = "Layer 1";
    TEArray<TEVector4> Pixels; // GridWidth * GridHeight
    float Opacity = 1.0f;
    bool Visible = true;
    bool Locked = false;
};

struct PixelFrame
{
    TEArray<PixelLayer> Layers;
    float Duration = 0.1f; // in seconds (for animation timing)
};

struct SpriteModeState
{
    TEArray<VectorElement> VectorElements;
    TEString ProcBuffer;
    TEArray<CustomKeyword> Keywords;
    TEArray<PixelFrame> PixelFrames;
    int ActiveFrameIndex = 0;
    int ActiveLayerIndex = 0;
};

inline bool EqualsIgnoreCase(const TEString &a, const TEString &b)
{
    if (a.length() != b.length())
        return false;
    for (size_t i = 0; i < a.length(); i++)
    {
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    }
    return true;
}

inline bool StartsWithIgnoreCase(const TEString &str, const TEString &prefix)
{
    if (str.length() < prefix.length())
        return false;
    for (size_t i = 0; i < prefix.length(); i++)
    {
        if (tolower(str[i]) != tolower(prefix[i]))
            return false;
    }
    return true;
}

inline size_t FindIgnoreCase(const TEString &str, const TEString &target)
{
    if (str.length() < target.length())
        return TEString::npos;
    for (size_t i = 0; i <= str.length() - target.length(); i++)
    {
        bool match = true;
        for (size_t j = 0; j < target.length(); j++)
        {
            if (tolower(str[i + j]) != tolower(target[j]))
            {
                match = false;
                break;
            }
        }
        if (match)
            return i;
    }
    return TEString::npos;
}


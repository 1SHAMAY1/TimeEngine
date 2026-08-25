#pragma once

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Utils/TEString.hpp"


class TE_API FontFunctionLibrary : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "FontFunctionLibrary";

    // ── Gameplay Static Methods ───────────────────────────────────────────
    static TERef<FontAsset> GetFont(AssetHandle fontHandle);
    static TEVector2 MeasureText(AssetHandle fontHandle, const TEString &text, float scale = 1.0f);
    static float MeasureTextWidth(AssetHandle fontHandle, const TEString &text, float scale = 1.0f);
    static float GetFontLineHeight(AssetHandle fontHandle, float scale = 1.0f);

    // ── Editor & Creation Static Methods ──────────────────────────────────
    static AssetHandle ImportFontFromTTF(const TEString &ttfPath, const TEString &saveAssetPath,
                                         float pixelSize = 32.0f, uint32_t atlasSize = 512);
};


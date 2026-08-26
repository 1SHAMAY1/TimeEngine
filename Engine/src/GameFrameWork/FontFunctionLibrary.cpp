#include "GameFrameWork/FontFunctionLibrary.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"

TERef<FontAsset> FontFunctionLibrary::GetFont(AssetHandle fontHandle)
{
    return AssetManager::GetAsset<FontAsset>(fontHandle);
}

TEVector2 FontFunctionLibrary::MeasureText(AssetHandle fontHandle, const TEString &text, float scale)
{
    auto font = GetFont(fontHandle);
    if (font)
    {
        return font->MeasureString(text, scale);
    }
    return TEVector2(0.0f, 0.0f);
}

float FontFunctionLibrary::MeasureTextWidth(AssetHandle fontHandle, const TEString &text, float scale)
{
    auto font = GetFont(fontHandle);
    if (font)
    {
        return font->MeasureStringWidth(text, scale);
    }
    return 0.0f;
}

float FontFunctionLibrary::GetFontLineHeight(AssetHandle fontHandle, float scale)
{
    auto font = GetFont(fontHandle);
    if (font)
    {
        return font->GetLineHeight() * scale;
    }
    return 0.0f;
}

AssetHandle FontFunctionLibrary::ImportFontFromTTF(const TEString &ttfPath, const TEString &saveAssetPath,
                                                   float pixelSize, uint32_t atlasSize)
{
    auto font = CreateRef<FontAsset>(saveAssetPath.GetStem(), pixelSize);
    if (font->BakeFromTTF(ttfPath, pixelSize, atlasSize, atlasSize))
    {
        if (font->SaveToFile(saveAssetPath))
        {
            AssetHandle handle = AssetRegistry::RegisterPath(saveAssetPath);
            font->SetHandle(handle);
            AssetManager::AddAsset(handle, font);
            return handle;
        }
    }
    return 0;
}

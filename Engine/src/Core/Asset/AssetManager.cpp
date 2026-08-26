#include "Core/PreRequisites.h"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <imstb_truetype.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Renderer/Texture.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>
#include <unordered_set>

TEMap<AssetHandle, TERef<Asset>> AssetManager::s_LoadedAssets;

static TEMap<TEString, AssetTypeMetadata> &GetAssetTypeRegistryMap()
{
    static TEMap<TEString, AssetTypeMetadata> s_Registry;
    return s_Registry;
}

const TEMap<TEString, AssetTypeMetadata> &AssetManager::GetRegisteredAssetTypes() { return GetAssetTypeRegistryMap(); }

#include "Core/Asset/DataAsset.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Core/Asset/StateTreeAsset.hpp"
#include "Core/Asset/TEDataTable.hpp"
#include "Core/Asset/TEStringTable.hpp"

void AssetManager::Init()
{
    TE_CORE_INFO("AssetManager initializing...");
    RegisterAssetType(CreateRef<DataAsset>());
    RegisterAssetType(CreateRef<TEDataTable>());
    RegisterAssetType(CreateRef<TEStringTable>());
    RegisterAssetType(CreateRef<FontAsset>());
    RegisterAssetType(CreateRef<StateTreeAsset>());
}

void AssetManager::Shutdown()
{
    TE_CORE_INFO("AssetManager shutting down...");
    s_LoadedAssets.Clear();
    GetAssetTypeRegistryMap().Clear();
}

void AssetManager::AddAsset(AssetHandle handle, const TERef<Asset> &asset)
{
    if (s_LoadedAssets.Find(handle) != nullptr)
    {
        TE_CORE_WARN("AssetManager: Overwriting asset with handle {0}", handle);
    }
    s_LoadedAssets[handle] = asset;
}

static TEString GetRootPath()
{
    static TEString s_RootPath = "";
    if (!s_RootPath.empty())
        return s_RootPath;

    TEString current = TEFileSystem::GetCurrentWorkingDirectory();
    while (current.HasParentPath())
    {
        if (TEFileSystem::Exists(current / "Resources"))
        {
            s_RootPath = current;
            return s_RootPath;
        }
        current = current.GetParentPath();
    }
    return "";
}

bool AssetManager::HasAsset(AssetHandle handle) { return s_LoadedAssets.Find(handle) != nullptr; }

void AssetManager::UnloadAsset(AssetHandle handle)
{
    if (s_LoadedAssets.Find(handle) != nullptr)
    {
        TE_CORE_INFO("AssetManager: Unloading asset handle {0}", handle);
        s_LoadedAssets.Remove(handle);
    }
}

bool AssetManager::DeleteAsset(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_WARN("AssetManager::DeleteAsset - path does not exist: {0}", path);
        return false;
    }

    // Resolve handle if registered
    if (AssetRegistry::Exists(path))
    {
        AssetHandle handle = AssetRegistry::RegisterPath(path); // returns existing handle
        UnloadAsset(handle);
        AssetRegistry::Unregister(handle);
    }

    if (!TEFileSystem::Remove(path))
    {
        TE_CORE_ERROR("AssetManager::DeleteAsset - failed to remove file: {0}", path);
        return false;
    }

    TE_CORE_INFO("AssetManager: Deleted asset at {0}", path);
    return true;
}

AssetHandle AssetManager::ReloadAsset(AssetHandle handle)
{
    TEString path = AssetRegistry::GetPath(handle);
    if (path.empty())
    {
        TE_CORE_WARN("AssetManager::ReloadAsset - no path registered for handle {0}", handle);
        return 0;
    }

    UnloadAsset(handle);
    return LoadAsset(path);
}

AssetHandle AssetManager::LoadAsset(const TEString &path)
{
    TEString finalPath = path;

    // Resolve relative paths starting with Resources/
    if (!TEFileSystem::Exists(finalPath) && (path.StartsWith("Resources/") || path.StartsWith("Resources\\")))
    {
        TEString root = GetRootPath();
        if (!root.empty())
        {
            finalPath = root / path;
        }
    }

    // --- CACHE CHECK FIRST ---
    AssetHandle handle = AssetRegistry::RegisterPath(finalPath);
    if (HasAsset(handle))
    {
        return handle;
    }

    // Keep track of failed loads to avoid repeatedly hitting disk and logging every frame
    static TESet<TEString> s_FailedLoads;
    if (s_FailedLoads.find(finalPath) != s_FailedLoads.end())
    {
        return 0;
    }

    if (!TEFileSystem::Exists(finalPath))
    {
        TE_CORE_ERROR("AssetManager: Failed to find asset at path: {0}", finalPath);
        s_FailedLoads.insert(finalPath);
        return 0;
    }

    TE_CORE_INFO("AssetManager: Loading asset from path {0}", finalPath.c_str());

    // Dynamic registered asset type dispatch (Fully modular asset loader)
    TEString ext = finalPath.GetExtension();
    for (const auto &[type, meta] : GetAssetTypeRegistryMap())
    {
        if ((meta.Extension == ext || (ext == ".jpg" && meta.Extension == ".png")) && meta.Prototype)
        {
            auto newAsset = CreateFromPrototype(ext);
            if (newAsset && newAsset->LoadFromFile(finalPath))
            {
                AddAsset(newAsset->GetHandle(), newAsset);
                return newAsset->GetHandle();
            }
        }
    }

    // If all load attempts failed, cache this path as a failed load to prevent retries
    s_FailedLoads.insert(finalPath);
    return 0;
}

TERef<Asset> AssetManager::CreateFromPrototype(const TEString &extension)
{
    for (const auto &[type, meta] : GetAssetTypeRegistryMap())
    {
        if ((meta.Extension == extension || (extension == ".jpg" && meta.Extension == ".png")) && meta.Prototype)
        {
            return meta.Prototype->Clone();
        }
    }
    return nullptr;
}

void AssetManager::RegisterAssetType(TERef<Asset> prototype)
{
    if (!prototype)
        return;

    AssetTypeMetadata metadata;
    metadata.Type = prototype->GetType();
    metadata.Extension = prototype->GetDefaultExtension();
    metadata.IconPath = prototype->GetDefaultIconPath();
    metadata.IconSize = prototype->GetDefaultIconSize();
    metadata.Prototype = prototype;

    GetAssetTypeRegistryMap()[metadata.Type] = metadata;
    TE_CORE_INFO("AssetManager: Registered asset type {0} (.{1}) -> {2}", metadata.Type, metadata.Extension,
                 metadata.IconPath);
}

TERef<class Texture> AssetManager::GetDefaultIcon(const TEString &type)
{
    auto &registry = GetAssetTypeRegistryMap();
    auto *found = registry.Find(type);
    if (found)
    {
        AssetHandle handle = LoadAsset(found->IconPath);
        return GetAsset<Texture>(handle);
    }
    return nullptr;
}

TERef<class Texture> AssetManager::GetIconForExtension(const TEString &extension)
{
    for (const auto &[type, entry] : GetAssetTypeRegistryMap())
    {
        if (entry.Extension == extension)
        {
            return GetDefaultIcon(type);
        }
    }
    return nullptr;
}

TEVector2 AssetManager::GetDefaultIconSize(const TEString &type)
{
    auto &registry = GetAssetTypeRegistryMap();
    auto *found = registry.Find(type);
    if (found)
        return found->IconSize;
    return {64.0f, 64.0f};
}

ImageData AssetManager::ImportImage(const TEString &filepath, int desiredChannels)
{
    ImageData img;
    stbi_set_flip_vertically_on_load(0);
    int w = 0, h = 0, c = 0;
    auto rawPixels = stbi_load(filepath.c_str(), &w, &h, &c, desiredChannels);
    if (rawPixels)
    {
        img.Width = w;
        img.Height = h;
        img.Channels = (desiredChannels > 0) ? desiredChannels : c;
        size_t totalBytes = static_cast<size_t>(img.Width * img.Height * img.Channels);
        img.Pixels.Reserve(totalBytes);
        for (size_t i = 0; i < totalBytes; ++i)
        {
            img.Pixels.Add(rawPixels[i]);
        }
        stbi_image_free(rawPixels);
    }
    return img;
}

bool AssetManager::ExportImagePNG(const TEString &path, int width, int height, int channels, const void *data)
{
    // Create directory if not exists
    if (path.HasParentPath())
    {
        TEFileSystem::CreateDirectories(path.GetParentPath());
    }

    int result = stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    if (result == 0)
    {
        TE_CORE_ERROR("Failed to save PNG: {0}", path);
        return false;
    }
    TE_CORE_INFO("Successfully saved PNG to {0}", path);
    return true;
}

bool AssetManager::BakeFontAtlas(const TEArray<uint8_t> &ttfData, float pixelSize, uint32_t atlasWidth,
                                 uint32_t atlasHeight, float &outAscent, float &outDescent, float &outLineHeight,
                                 TEMap<TEString, FontGlyph> &outGlyphs, TEArray<uint8_t> &outRgbaBitmap)
{
    if (ttfData.IsEmpty())
        return false;

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, ttfData.GetData(), stbtt_GetFontOffsetForIndex(ttfData.GetData(), 0)))
    {
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelSize);
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    outAscent = ascent * scale;
    outDescent = descent * scale;
    outLineHeight = (ascent - descent + lineGap) * scale;

    const int firstChar = 32;
    const int numChars = 96;
    TEArray<stbtt_bakedchar> bakedChars;
    bakedChars.Resize(numChars);
    TEArray<uint8_t> monoBitmap;
    monoBitmap.Resize(atlasWidth * atlasHeight, 0);

    int result = stbtt_BakeFontBitmap(ttfData.GetData(), 0, pixelSize, monoBitmap.GetData(), atlasWidth, atlasHeight,
                                      firstChar, numChars, bakedChars.GetData());
    if (result <= 0)
    {
        TE_CORE_WARN("AssetManager::BakeFontAtlas - Atlas might be too small for font size!");
    }

    outGlyphs.clear();
    for (int i = 0; i < numChars; ++i)
    {
        char ch = static_cast<char>(firstChar + i);
        TEString chStr(&ch, 1);
        const auto &bc = bakedChars[i];

        FontGlyph glyph;
        glyph.Character = chStr;
        glyph.AdvanceX = bc.xadvance;
        glyph.BearingX = bc.xoff;
        glyph.BearingY = bc.yoff;
        glyph.Width = static_cast<float>(bc.x1 - bc.x0);
        glyph.Height = static_cast<float>(bc.y1 - bc.y0);

        glyph.UV.x = static_cast<float>(bc.x0) / atlasWidth;
        glyph.UV.y = static_cast<float>(bc.y0) / atlasHeight;
        glyph.UV.z = static_cast<float>(bc.x1) / atlasWidth;
        glyph.UV.w = static_cast<float>(bc.y1) / atlasHeight;

        outGlyphs[chStr] = glyph;
    }

    outRgbaBitmap.Resize(atlasWidth * atlasHeight * 4);
    for (size_t i = 0; i < monoBitmap.Num(); ++i)
    {
        outRgbaBitmap[i * 4 + 0] = 255;
        outRgbaBitmap[i * 4 + 1] = 255;
        outRgbaBitmap[i * 4 + 2] = 255;
        outRgbaBitmap[i * 4 + 3] = monoBitmap[i];
    }

    return true;
}

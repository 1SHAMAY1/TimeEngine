#include "PreRequisites.h"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

#include "AssetManager.hpp"
#include "FontAsset.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <imstb_truetype.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "Asset.hpp"
#include "AssetRegistry.hpp"
#include "Log.h"
#include "Texture.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <sstream>
#include <unordered_set>

#include <mutex>

TEMap<AssetHandle, TERef<Asset>> AssetManager::s_LoadedAssets;
static std::mutex s_AssetTypeRegistryMutex;

TERef<Asset> AssetManager::GetAssetRaw(AssetHandle handle)
{
    auto *found = s_LoadedAssets.Find(handle);
    if (found)
    {
        return *found;
    }
    return nullptr;
}

static TEMap<TEString, AssetTypeMetadata> &GetAssetTypeRegistryMap()
{
    static TEMap<TEString, AssetTypeMetadata> s_Registry;
    return s_Registry;
}

const TEMap<TEString, AssetTypeMetadata> &AssetManager::GetRegisteredAssetTypes()
{
    std::lock_guard<std::mutex> lock(s_AssetTypeRegistryMutex);
    return GetAssetTypeRegistryMap();
}

#include "DataAsset.hpp"
#include "FontAsset.hpp"
#include "StateTreeAsset.hpp"
#include "TEDataTable.hpp"
#include "TEStringTable.hpp"

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

    {
        std::lock_guard<std::mutex> lock(s_AssetTypeRegistryMutex);
        GetAssetTypeRegistryMap()[metadata.Type] = metadata;
    }
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
        TE_CORE_INFO("AssetManager::ImportImage: Imported '", filepath, "' (", w, "x", h, ", ", img.Channels, " channels, ", (totalBytes / 1024), " KB)");
    }
    else
    {
        TE_CORE_WARN("AssetManager::ImportImage: Failed to import image '", filepath, "' (Reason: ", (stbi_failure_reason() ? stbi_failure_reason() : "Unknown"), ")");
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

DDSHeaderInfo AssetManager::ParseDDSHeader(const TEArray<uint8_t> &ddsBytes)
{
    DDSHeaderInfo info;
    info.IsValid = false;

    // Minimum DDS header is 128 bytes (4 magic + 124 header)
    if (ddsBytes.Num() < 128)
        return info;

    const auto pBytes = ddsBytes.GetData();

    // Verify magic 'DDS ' (0x20534444)
    uint32_t magic = *reinterpret_cast<const uint32_t *>(pBytes);
    if (magic != 0x20534444)
        return info;

    uint32_t headerSize = *reinterpret_cast<const uint32_t *>(pBytes + 4);
    if (headerSize != 124)
        return info;

    info.Height = *reinterpret_cast<const uint32_t *>(pBytes + 12);
    info.Width = *reinterpret_cast<const uint32_t *>(pBytes + 16);
    info.LinearSize = *reinterpret_cast<const uint32_t *>(pBytes + 20);
    info.MipMapCount = *reinterpret_cast<const uint32_t *>(pBytes + 28);
    if (info.MipMapCount == 0)
        info.MipMapCount = 1;

    // Pixel format flags and FourCC
    uint32_t pfFlags = *reinterpret_cast<const uint32_t *>(pBytes + 80);
    uint32_t fourCC = *reinterpret_cast<const uint32_t *>(pBytes + 84);

    info.DataOffset = 128;
    info.Format = DDSCompressionFormat::Unknown;

    if (pfFlags & 0x4) // DDPF_FOURCC
    {
        if (fourCC == 0x31545844) // "DXT1"
            info.Format = DDSCompressionFormat::BC1_DXT1;
        else if (fourCC == 0x33545844) // "DXT3"
            info.Format = DDSCompressionFormat::BC2_DXT3;
        else if (fourCC == 0x35545844) // "DXT5"
            info.Format = DDSCompressionFormat::BC3_DXT5;
        else if (fourCC == 0x31495441 || fourCC == 0x55344342) // "ATI1" or "BC4U"
            info.Format = DDSCompressionFormat::BC4_UNORM;
        else if (fourCC == 0x32495441 || fourCC == 0x55354342) // "ATI2" or "BC5U"
            info.Format = DDSCompressionFormat::BC5_UNORM;
        else if (fourCC == 0x30315844) // "DX10"
        {
            if (ddsBytes.Num() >= 148)
            {
                info.DataOffset = 148;
                uint32_t dxgiFormat = *reinterpret_cast<const uint32_t *>(pBytes + 128);
                if (dxgiFormat == 71 || dxgiFormat == 72)
                    info.Format = DDSCompressionFormat::BC1_DXT1;
                else if (dxgiFormat == 74 || dxgiFormat == 75)
                    info.Format = DDSCompressionFormat::BC2_DXT3;
                else if (dxgiFormat == 77 || dxgiFormat == 78)
                    info.Format = DDSCompressionFormat::BC3_DXT5;
                else if (dxgiFormat == 80)
                    info.Format = DDSCompressionFormat::BC4_UNORM;
                else if (dxgiFormat == 83)
                    info.Format = DDSCompressionFormat::BC5_UNORM;
                else if (dxgiFormat == 98 || dxgiFormat == 99)
                    info.Format = DDSCompressionFormat::BC7_UNORM;
                else if (dxgiFormat == 28)
                    info.Format = DDSCompressionFormat::RGBA8_UNORM;
            }
        }
    }
    else if (pfFlags & 0x40) // DDPF_RGB uncompressed
    {
        uint32_t rgbBitCount = *reinterpret_cast<const uint32_t *>(pBytes + 88);
        if (rgbBitCount == 32)
            info.Format = DDSCompressionFormat::RGBA8_UNORM;
    }

    info.IsValid = (info.Width > 0 && info.Height > 0 && info.Format != DDSCompressionFormat::Unknown);
    return info;
}

DDSHeaderInfo AssetManager::ParseDDSFile(const TEString &filepath)
{
    if (!TEFileSystem::Exists(filepath))
    {
        DDSHeaderInfo info;
        info.IsValid = false;
        return info;
    }
    TEArray<uint8_t> bytes = TEFileSystem::ReadAllBytes(filepath);
    return ParseDDSHeader(bytes);
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

#include "TextureSerializer.hpp"

bool AssetManager::ImportAsset(const AssetImportConfig &config, TEString &outCreatedAssetPath)
{
    if (config.SourceFilePath.IsEmpty() || !TEFileSystem::Exists(config.SourceFilePath))
    {
        TE_CORE_ERROR("AssetManager::ImportAsset: Source file '{0}' does not exist", config.SourceFilePath.c_str());
        return false;
    }

    if (!TEFileSystem::Exists(config.TargetDirectory))
    {
        TEFileSystem::CreateDirectories(config.TargetDirectory);
    }

    TEString sourceExt = config.SourceFilePath.GetExtension();
    TEString targetName = config.TargetAssetName.empty() ? config.SourceFilePath.GetStem() : config.TargetAssetName;
    TEString targetExt = config.TargetExtension;

    // 1. Copy & rename source file to match target asset name in the target directory
    TEString copiedSourceName = targetName + sourceExt;
    TEString targetSourcePath = config.TargetDirectory / copiedSourceName;

    TEString normSource = TEFileSystem::GetAbsolutePath(config.SourceFilePath);
    TEString normTargetSource = TEFileSystem::GetAbsolutePath(targetSourcePath);

    if (normSource != normTargetSource)
    {
        if (!TEFileSystem::CopyFile(config.SourceFilePath, targetSourcePath, true))
        {
            TE_CORE_ERROR("AssetManager::ImportAsset: Failed to copy source file from '{0}' to '{1}'",
                          config.SourceFilePath.c_str(), targetSourcePath.c_str());
            return false;
        }
    }

    // 2. Generate engine asset metadata
    if (targetExt.Equals(".tetexture", ESearchCase::IgnoreCase))
    {
        outCreatedAssetPath = config.TargetDirectory / (targetName + ".tetexture");

        auto tex = CreateRef<Texture>(targetSourcePath);
        if (tex)
        {
            tex->SetName(targetName);
            tex->SetFilePath(copiedSourceName);
            tex->SetFilterMode((config.TextureFilterMode == 1) ? TextureFilterMode::Nearest : TextureFilterMode::Linear);
            
            TextureWrapMode wrap = TextureWrapMode::Repeat;
            if (config.TextureWrapMode == 1)
                wrap = TextureWrapMode::ClampToEdge;
            else if (config.TextureWrapMode == 2)
                wrap = TextureWrapMode::MirroredRepeat;
            tex->SetWrapMode(wrap);

            tex->SetGenerateMipmaps(config.GenerateMipmaps);
            tex->SetPremultipliedAlpha(config.PremultipliedAlpha);

            TextureSerializer serializer(tex);
            if (!serializer.Serialize(outCreatedAssetPath))
            {
                TE_CORE_ERROR("AssetManager::ImportAsset: Failed to serialize texture to '{0}'", outCreatedAssetPath.c_str());
                return false;
            }
        }

        else
        {
            // Direct write fallback
            TEString filterStr = (config.TextureFilterMode == 1) ? "Nearest" : "Linear";
            TEString wrapStr = (config.TextureWrapMode == 1) ? "ClampToEdge" : ((config.TextureWrapMode == 2) ? "MirroredRepeat" : "Repeat");
            TEString content = "Texture2D: " + targetName + "\n";
            content += "ImagePath: " + copiedSourceName + "\n";
            content += "FilterMode: " + filterStr + "\n";
            content += "WrapMode: " + wrapStr + "\n";
            content += "GenerateMipmaps: " + TEString(config.GenerateMipmaps ? "true" : "false") + "\n";
            content += "PremultipliedAlpha: " + TEString(config.PremultipliedAlpha ? "true" : "false") + "\n";
            TEFileSystem::WriteAllText(outCreatedAssetPath, content);
        }
    }
    else if (targetExt.Equals(".tefont", ESearchCase::IgnoreCase))
    {
        outCreatedAssetPath = config.TargetDirectory / (targetName + ".tefont");
        auto font = CreateRef<FontAsset>();
        if (font)
        {
            font->SetName(targetName);
            font->BakeFromTTF(targetSourcePath, config.FontSize, config.AtlasWidth, config.AtlasHeight);
            font->SaveToFile(outCreatedAssetPath);
        }
    }
    else if (targetExt.Equals(".tedatatable", ESearchCase::IgnoreCase))
    {
        outCreatedAssetPath = config.TargetDirectory / (targetName + ".tedatatable");
        auto table = CreateRef<TEDataTable>();
        if (table)
        {
            table->SetName(targetName);
            table->ImportFromCSV(targetSourcePath);
            table->SaveToFile(outCreatedAssetPath);
        }
    }
    else if (targetExt.Equals(".testringtable", ESearchCase::IgnoreCase))
    {
        outCreatedAssetPath = config.TargetDirectory / (targetName + ".testringtable");
        auto strTable = CreateRef<TEStringTable>();
        if (strTable)
        {
            strTable->SetName(targetName);
            strTable->ImportFromCSV(targetSourcePath);
            strTable->SaveToFile(outCreatedAssetPath);
        }
    }
    else
    {
        outCreatedAssetPath = config.TargetDirectory / (targetName + targetExt);
    }

    TE_CORE_INFO("AssetManager::ImportAsset: Successfully imported '{0}' as '{1}'",
                 config.SourceFilePath.c_str(), outCreatedAssetPath.c_str());
    return true;
}

bool AssetManager::ReimportAsset(const TEString &assetPath)
{
    if (!TEFileSystem::Exists(assetPath))
        return false;

    TEString ext = assetPath.GetExtension();
    if (ext.Equals(".tetexture", ESearchCase::IgnoreCase))
    {
        auto tex = CreateRef<Texture>();
        TextureSerializer serializer(tex);
        if (serializer.Deserialize(assetPath))
        {
            // Re-apply to any cached loaded asset
            for (auto &pair : s_LoadedAssets)
            {
                if (pair.second && pair.second->GetAssetPath() == assetPath)
                {
                    pair.second->LoadFromFile(assetPath);
                }
            }
            TE_CORE_INFO("AssetManager: Successfully reimported texture '{0}'", assetPath.c_str());
            return true;
        }
    }
    else if (ext.Equals(".tefont", ESearchCase::IgnoreCase))
    {
        auto font = CreateRef<FontAsset>();
        if (font->LoadFromFile(assetPath))
        {
            TEString src = font->GetSourcePath();
            if (!src.empty() && TEFileSystem::Exists(src))
            {
                font->BakeFromTTF(src, font->GetPixelSize(), font->GetAtlasWidth(), font->GetAtlasHeight());
                font->SaveToFile(assetPath);
                TE_CORE_INFO("AssetManager: Successfully reimported font '{0}'", assetPath.c_str());
                return true;
            }
        }
    }

    return true;
}


bool AssetManager::ReimportAssetWithNewSource(const TEString &assetPath, const TEString &newSourcePath)
{
    if (!TEFileSystem::Exists(assetPath) || !TEFileSystem::Exists(newSourcePath))
        return false;

    TEString assetDir = assetPath.GetParentPath();
    TEString assetStem = assetPath.GetStem();
    TEString newSourceExt = newSourcePath.GetExtension();

    // Overwrite local source file with the new source
    TEString targetSourcePath = assetDir / (assetStem + newSourceExt);
    if (!TEFileSystem::CopyFile(newSourcePath, targetSourcePath, true))
    {
        TE_CORE_ERROR("AssetManager::ReimportAssetWithNewSource: Failed to copy '{0}' to '{1}'",
                      newSourcePath.c_str(), targetSourcePath.c_str());
        return false;
    }

    return ReimportAsset(assetPath);
}


#pragma once
#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"


struct TE_API ImageData
{
    TEArray<uint8_t> Pixels;
    int Width = 0;
    int Height = 0;
    int Channels = 0;

    bool IsValid() const { return !Pixels.IsEmpty() && Width > 0 && Height > 0; }
    void *Data() { return Pixels.GetData(); }
    const void *Data() const { return Pixels.GetData(); }
    const void *GetData() const { return Pixels.GetData(); }
};

struct AssetTypeMetadata
{
    TEString Type;
    TEString Extension;
    TEString IconPath;
    TEVector2 IconSize = {64.0f, 64.0f};
    TERef<Asset> Prototype;
};

class TE_API AssetManager
{
public:
    static void Init();
    static void Shutdown();

    template <typename T> static TERef<T> GetAsset(AssetHandle handle)
    {
        auto *found = s_LoadedAssets.Find(handle);
        if (found)
        {
            return std::static_pointer_cast<T>(*found);
        }
        return nullptr;
    }

    static AssetHandle LoadAsset(const TEString &path);
    static AssetHandle ReloadAsset(AssetHandle handle);
    static void        UnloadAsset(AssetHandle handle);
    static bool        DeleteAsset(const TEString &path);

    static void AddAsset(AssetHandle handle, const TERef<Asset> &asset);
    static bool HasAsset(AssetHandle handle);

    // Modular Registration
    static void RegisterAssetType(TERef<Asset> prototype);
    static const TEMap<TEString, AssetTypeMetadata> &GetRegisteredAssetTypes();
    static TERef<Asset> CreateFromPrototype(const TEString &extension);

    static TERef<class Texture> GetDefaultIcon(const TEString &type);
    static TERef<class Texture> GetIconForExtension(const TEString &extension);
    static TEVector2 GetDefaultIconSize(const TEString &type);

    // Image & Font utilities (stb encapsulation)
    static ImageData ImportImage(const TEString &filepath, int desiredChannels = 0);
    static bool ExportImagePNG(const TEString &path, int width, int height, int channels, const void *data);
    static bool BakeFontAtlas(const TEArray<uint8_t> &ttfData, float pixelSize,
                              uint32_t atlasWidth, uint32_t atlasHeight,
                              float &outAscent, float &outDescent, float &outLineHeight,
                              TEMap<TEString, struct FontGlyph> &outGlyphs,
                              TEArray<uint8_t> &outRgbaBitmap);

private:
    static TEMap<AssetHandle, TERef<Asset>> s_LoadedAssets;
};

#define TE_REGISTER_ASSET(type) \
    static struct TEAssetRegistrar_##type { \
        TEAssetRegistrar_##type() { AssetManager::RegisterAssetType(CreateRef<type>()); } \
    } g_TEAssetRegistrar_##type;


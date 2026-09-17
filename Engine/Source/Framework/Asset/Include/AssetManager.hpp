#pragma once
#include "Asset.hpp"
#include "GameplayUtils.hpp"
#include "MathUtils.hpp"

enum class DDSCompressionFormat
{
    Unknown = 0,
    BC1_DXT1,
    BC2_DXT3,
    BC3_DXT5,
    BC4_UNORM,
    BC5_UNORM,
    BC7_UNORM,
    RGBA8_UNORM
};

struct TE_API DDSHeaderInfo
{
    bool IsValid = false;
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t MipMapCount = 0;
    DDSCompressionFormat Format = DDSCompressionFormat::Unknown;
    size_t DataOffset = 0;
    size_t LinearSize = 0;
};

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

struct AssetImportConfig
{
    TEString SourceFilePath;
    TEString TargetDirectory;
    TEString TargetAssetName;
    TEString TargetExtension;
    TEString AssetType;

    // Texture Specific Options
    int TextureFilterMode = 0;   // 0: Linear, 1: Nearest
    int TextureWrapMode = 0;     // 0: Repeat, 1: ClampToEdge, 2: MirroredRepeat
    bool GenerateMipmaps = false;
    bool PremultipliedAlpha = false;

    // Font Specific Options
    float FontSize = 32.0f;
    uint32_t AtlasWidth = 1024;
    uint32_t AtlasHeight = 1024;

    // Table Options
    bool IsDataTable = true;
};

class TE_API AssetManager
{
public:
    static void Init();
    static void Shutdown();

    static TERef<Asset> GetAssetRaw(AssetHandle handle);

    template <typename T> static TERef<T> GetAsset(AssetHandle handle)
    {
        auto asset = GetAssetRaw(handle);
        if (asset)
        {
            return std::static_pointer_cast<T>(asset);
        }
        return nullptr;
    }

    static AssetHandle LoadAsset(const TEString &path);
    static AssetHandle ReloadAsset(AssetHandle handle);
    static void UnloadAsset(AssetHandle handle);
    static bool DeleteAsset(const TEString &path);

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
    static DDSHeaderInfo ParseDDSHeader(const TEArray<uint8_t> &ddsBytes);
    static DDSHeaderInfo ParseDDSFile(const TEString &filepath);
    static bool BakeFontAtlas(const TEArray<uint8_t> &ttfData, float pixelSize, uint32_t atlasWidth,
                              uint32_t atlasHeight, float &outAscent, float &outDescent, float &outLineHeight,
                              TEMap<TEString, struct FontGlyph> &outGlyphs, TEArray<uint8_t> &outRgbaBitmap);

    // High-Level Import & Reimport Engine Operations
    static bool ImportAsset(const AssetImportConfig &config, TEString &outCreatedAssetPath);
    static bool ReimportAsset(const TEString &assetPath);
    static bool ReimportAssetWithNewSource(const TEString &assetPath, const TEString &newSourcePath);

private:
    static TEMap<AssetHandle, TERef<Asset>> s_LoadedAssets;
};


#define TE_REGISTER_ASSET(type)                                                                                        \
    static struct TEAssetRegistrar_##type                                                                              \
    {                                                                                                                  \
        TEAssetRegistrar_##type() { AssetManager::RegisterAssetType(CreateRef<type>()); }                              \
    } g_TEAssetRegistrar_##type;

#include "Core/Asset/AssetManager.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/Scene/Scene.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/MaterialSerializer.hpp"
#include "Renderer/Sprite.hpp"
#include "Renderer/SpriteSerializer.hpp"
#include "Renderer/SpriteSheet.hpp"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include <filesystem>
#include <unordered_set>

namespace TE
{

std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_LoadedAssets;
std::unordered_map<std::string, AssetTypeMetadata> AssetManager::s_AssetTypeRegistry;

void AssetManager::Init()
{
    TE_CORE_INFO("AssetManager initializing...");

    // Explicitly register core asset types via prototypes
    RegisterAssetType(std::make_shared<Scene>());
    RegisterAssetType(std::make_shared<Texture>("Resources/Editor/TextureIcon.png"));
    RegisterAssetType(std::make_shared<Material>(nullptr));
    RegisterAssetType(std::make_shared<Sprite>());
    RegisterAssetType(std::make_shared<SpriteSheet>());
}

void AssetManager::Shutdown()
{
    TE_CORE_INFO("AssetManager shutting down...");
    s_LoadedAssets.clear();
    s_AssetTypeRegistry.clear();
}

void AssetManager::AddAsset(AssetHandle handle, const std::shared_ptr<Asset> &asset)
{
    if (s_LoadedAssets.find(handle) != s_LoadedAssets.end())
    {
        TE_CORE_WARN("AssetManager: Overwriting asset with handle {0}", handle);
    }
    s_LoadedAssets[handle] = asset;
}

static std::filesystem::path GetRootPath()
{
    static std::filesystem::path s_RootPath = "";
    if (!s_RootPath.empty())
        return s_RootPath;

    std::filesystem::path current = std::filesystem::current_path();
    while (current.has_parent_path())
    {
        if (std::filesystem::exists(current / "Resources"))
        {
            s_RootPath = current;
            return s_RootPath;
        }
        current = current.parent_path();
    }
    return "";
}

bool AssetManager::HasAsset(AssetHandle handle) { return s_LoadedAssets.find(handle) != s_LoadedAssets.end(); }

AssetHandle AssetManager::LoadAsset(const std::filesystem::path &path)
{
    std::filesystem::path finalPath = path;

    // Resolve relative paths starting with Resources/
    std::string pathStr = path.string();
    if (!std::filesystem::exists(finalPath) && (pathStr.find("Resources/") == 0 || pathStr.find("Resources\\") == 0))
    {
        std::filesystem::path root = GetRootPath();
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
    static std::unordered_set<std::wstring> s_FailedLoads;
    if (s_FailedLoads.find(finalPath.wstring()) != s_FailedLoads.end())
    {
        return 0;
    }

    if (!std::filesystem::exists(finalPath))
    {
        TE_CORE_ERROR("AssetManager: Failed to find asset at path: {0}", finalPath.string());
        s_FailedLoads.insert(finalPath.wstring());
        return 0;
    }

    TE_CORE_INFO("AssetManager: Loading asset from path {0}", finalPath.string());

    // If it's a texture, we can actually load it for icons etc.
    if (finalPath.extension() == ".png" || finalPath.extension() == ".jpg")
    {
        auto tex = std::make_shared<Texture>(finalPath.string());
        AddAsset(tex->GetHandle(), tex);
        return tex->GetHandle();
    }
    else if (finalPath.extension() == ".tematerial")
    {
        auto mat = std::make_shared<Material>(nullptr);
        MaterialSerializer serializer(mat);
        if (serializer.Deserialize(finalPath))
        {
            AddAsset(mat->GetHandle(), mat);
            return mat->GetHandle();
        }
    }
    else if (finalPath.extension() == ".tesprite")
    {
        auto sprite = std::make_shared<Sprite>();
        SpriteSerializer serializer(sprite);
        if (serializer.Deserialize(finalPath))
        {
            AddAsset(sprite->GetHandle(), sprite);
            return sprite->GetHandle();
        }
    }
    else if (finalPath.extension() == ".tespritesheet")
    {
        auto sheet = std::make_shared<SpriteSheet>();
        SpriteSheetSerializer serializer(sheet);
        if (serializer.Deserialize(finalPath))
        {
            AddAsset(sheet->GetHandle(), sheet);
            return sheet->GetHandle();
        }
    }
    else if (finalPath.extension() == ".tetexture")
    {
        auto tex = std::make_shared<Texture>(finalPath.string());
        TextureSerializer serializer(tex);
        if (serializer.Deserialize(finalPath))
        {
            AddAsset(tex->GetHandle(), tex);
            return tex->GetHandle();
        }
    }

    // If all load attempts failed, cache this path as a failed load to prevent retries
    s_FailedLoads.insert(finalPath.wstring());
    return 0; // AssetRegistry will handle the mapping later
}

void AssetManager::RegisterAssetType(std::shared_ptr<Asset> prototype)
{
    if (!prototype)
        return;

    AssetTypeMetadata metadata;
    metadata.Type = prototype->GetType();
    metadata.Extension = prototype->GetDefaultExtension();
    metadata.IconPath = prototype->GetDefaultIconPath();
    metadata.IconSize = prototype->GetDefaultIconSize();
    metadata.Prototype = prototype;

    s_AssetTypeRegistry[metadata.Type] = metadata;
    TE_CORE_INFO("AssetManager: Registered asset type {0} (.{1}) -> {2}", metadata.Type, metadata.Extension,
                 metadata.IconPath);
}

std::shared_ptr<class Texture> AssetManager::GetDefaultIcon(const std::string &type)
{
    auto it = s_AssetTypeRegistry.find(type);
    if (it != s_AssetTypeRegistry.end())
    {
        AssetHandle handle = LoadAsset(it->second.IconPath);
        return GetAsset<Texture>(handle);
    }
    return nullptr;
}

std::shared_ptr<class Texture> AssetManager::GetIconForExtension(const std::string &extension)
{
    for (const auto &[type, entry] : s_AssetTypeRegistry)
    {
        if (entry.Extension == extension)
        {
            return GetDefaultIcon(type);
        }
    }
    return nullptr;
}

TEVector2 AssetManager::GetDefaultIconSize(const std::string &type)
{
    auto it = s_AssetTypeRegistry.find(type);
    if (it != s_AssetTypeRegistry.end())
    {
        return it->second.IconSize;
    }
    return {64.0f, 64.0f};
}

ImageData AssetManager::ImportImage(const std::string &filepath, int desiredChannels)
{
    ImageData img;
    img.Data = stbi_load(filepath.c_str(), &img.Width, &img.Height, &img.Channels, desiredChannels);
    if (desiredChannels > 0)
    {
        img.Channels = desiredChannels;
    }
    return img;
}

void AssetManager::FreeImage(unsigned char *data)
{
    if (data)
    {
        stbi_image_free(data);
    }
}

bool AssetManager::ExportImagePNG(const std::string &path, int width, int height, int channels, const void *data)
{
    // Create directory if not exists
    std::filesystem::path p = path;
    if (p.has_parent_path())
    {
        std::filesystem::create_directories(p.parent_path());
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

} // namespace TE

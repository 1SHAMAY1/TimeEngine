#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Sprite.hpp"
#include "Renderer/SpriteSheet.hpp"
#include "Core/Scene/Scene.hpp"
#include <filesystem>

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

static std::filesystem::path GetRootPath() {
    static std::filesystem::path s_RootPath = "";
    if (!s_RootPath.empty()) return s_RootPath;

    std::filesystem::path current = std::filesystem::current_path();
    while (current.has_parent_path()) {
        if (std::filesystem::exists(current / "Resources")) {
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
    if (!std::filesystem::exists(finalPath)) {
        std::string pathStr = path.string();
        if (pathStr.find("Resources/") == 0 || pathStr.find("Resources\\") == 0) {
            std::filesystem::path root = GetRootPath();
            if (!root.empty()) {
                finalPath = root / path;
            }
        }
    }

    TE_CORE_INFO("AssetManager: Loading asset from path {0}", finalPath.string());
    
    if (!std::filesystem::exists(finalPath)) {
        TE_CORE_ERROR("AssetManager: Failed to find asset at path: {0}", finalPath.string());
        return 0;
    }

    // Cache check
    AssetHandle handle = AssetRegistry::RegisterPath(finalPath);
    if (HasAsset(handle)) {
        return handle;
    }

    // If it's a texture, we can actually load it for icons etc.
    if (finalPath.extension() == ".png" || finalPath.extension() == ".jpg") {
        auto tex = std::make_shared<Texture>(finalPath.string());
        AddAsset(tex->GetHandle(), tex);
        return tex->GetHandle();
    }

    return 0; // AssetRegistry will handle the mapping later
}

void AssetManager::RegisterAssetType(std::shared_ptr<Asset> prototype) {
    if (!prototype) return;

    AssetTypeMetadata metadata;
    metadata.Type = prototype->GetType();
    metadata.Extension = prototype->GetDefaultExtension();
    metadata.IconPath = prototype->GetDefaultIconPath();
    metadata.IconSize = prototype->GetDefaultIconSize();
    metadata.Prototype = prototype;

    s_AssetTypeRegistry[metadata.Type] = metadata;
    TE_CORE_INFO("AssetManager: Registered asset type {0} (.{1}) -> {2}", metadata.Type, metadata.Extension, metadata.IconPath);
}

std::shared_ptr<class Texture> AssetManager::GetDefaultIcon(const std::string& type) {
    auto it = s_AssetTypeRegistry.find(type);
    if (it != s_AssetTypeRegistry.end()) {
        AssetHandle handle = LoadAsset(it->second.IconPath);
        return GetAsset<Texture>(handle);
    }
    return nullptr;
}

std::shared_ptr<class Texture> AssetManager::GetIconForExtension(const std::string& extension) {
    for (const auto& [type, entry] : s_AssetTypeRegistry) {
        if (entry.Extension == extension) {
            return GetDefaultIcon(type);
        }
    }
    return nullptr;
}

TEVector2 AssetManager::GetDefaultIconSize(const std::string& type) {
    auto it = s_AssetTypeRegistry.find(type);
    if (it != s_AssetTypeRegistry.end()) {
        return it->second.IconSize;
    }
    return { 64.0f, 64.0f };
}

} // namespace TE

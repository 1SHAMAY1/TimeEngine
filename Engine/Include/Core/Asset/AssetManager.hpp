#pragma once
#include "Core/Asset/Asset.hpp"
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include "Utility/MathUtils.hpp"

namespace TE
{

struct AssetTypeMetadata {
    std::string Type;
    std::string Extension;
    std::string IconPath;
    TEVector2 IconSize = { 64.0f, 64.0f };
    std::shared_ptr<Asset> Prototype;
};

class AssetManager
{
public:
    static void Init();
    static void Shutdown();

    template <typename T> static std::shared_ptr<T> GetAsset(AssetHandle handle)
    {
        if (s_LoadedAssets.find(handle) != s_LoadedAssets.end())
        {
            return std::static_pointer_cast<T>(s_LoadedAssets[handle]);
        }
        return nullptr;
    }

    static AssetHandle LoadAsset(const std::filesystem::path &path);

    static void AddAsset(AssetHandle handle, const std::shared_ptr<Asset> &asset);
    static bool HasAsset(AssetHandle handle);

    // Modular Registration
    static void RegisterAssetType(std::shared_ptr<Asset> prototype);
    static const std::unordered_map<std::string, AssetTypeMetadata>& GetRegisteredAssetTypes() { return s_AssetTypeRegistry; }
    
    static std::shared_ptr<class Texture> GetDefaultIcon(const std::string& type);
    static std::shared_ptr<class Texture> GetIconForExtension(const std::string& extension);
    static TEVector2 GetDefaultIconSize(const std::string& type);

private:
    static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_LoadedAssets;
    static std::unordered_map<std::string, AssetTypeMetadata> s_AssetTypeRegistry;
};

} // namespace TE

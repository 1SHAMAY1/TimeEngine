#pragma once
#include "Core/Asset/Asset.hpp"
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace TE {

    class AssetManager {
    public:
        static void Init();
        static void Shutdown();

        template<typename T>
        static std::shared_ptr<T> GetAsset(AssetHandle handle) {
            if (s_LoadedAssets.find(handle) != s_LoadedAssets.end()) {
                return std::static_pointer_cast<T>(s_LoadedAssets[handle]);
            }
            return nullptr;
        }

        static AssetHandle LoadAsset(const std::filesystem::path& path);
        
        static void AddAsset(AssetHandle handle, const std::shared_ptr<Asset>& asset);
        static bool HasAsset(AssetHandle handle);

    private:
        static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_LoadedAssets;
    };

} // namespace TE

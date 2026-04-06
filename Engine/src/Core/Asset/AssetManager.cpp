#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"

namespace TE {

    std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_LoadedAssets;

    void AssetManager::Init() {
        TE_CORE_INFO("AssetManager initializing...");
    }

    void AssetManager::Shutdown() {
        TE_CORE_INFO("AssetManager shutting down...");
        s_LoadedAssets.clear();
    }

    void AssetManager::AddAsset(AssetHandle handle, const std::shared_ptr<Asset>& asset) {
        if (s_LoadedAssets.find(handle) != s_LoadedAssets.end()) {
            TE_CORE_WARN("AssetManager: Overwriting asset with handle {0}", handle);
        }
        s_LoadedAssets[handle] = asset;
    }

    bool AssetManager::HasAsset(AssetHandle handle) {
        return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
    }

    AssetHandle AssetManager::LoadAsset(const std::filesystem::path& path) {
        // Basic loading stub
        TE_CORE_INFO("AssetManager: Loading asset from path {0}", path.string());
        return 0; // AssetRegistry will handle the mapping later
    }

} // namespace TE

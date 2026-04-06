#pragma once
#include "Core/Asset/Asset.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

namespace TE
{

class AssetRegistry
{
public:
    static AssetHandle RegisterPath(const std::filesystem::path &path);
    static std::filesystem::path GetPath(AssetHandle handle);
    static bool Exists(AssetHandle handle);
    static bool Exists(const std::filesystem::path &path);

    static void Save(const std::filesystem::path &path);
    static void Load(const std::filesystem::path &path);

private:
    static std::unordered_map<AssetHandle, std::filesystem::path> s_HandleToPath;
    static std::unordered_map<std::string, AssetHandle> s_PathToHandle;
    static AssetHandle s_NextHandle;
};

} // namespace TE

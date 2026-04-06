#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include <fstream>
#include <sstream>

namespace TE {

    std::unordered_map<AssetHandle, std::filesystem::path> AssetRegistry::s_HandleToPath;
    std::unordered_map<std::string, AssetHandle> AssetRegistry::s_PathToHandle;
    AssetHandle AssetRegistry::s_NextHandle = 1;

    AssetHandle AssetRegistry::RegisterPath(const std::filesystem::path& path) {
        std::string pathStr = path.string();
        if (s_PathToHandle.find(pathStr) != s_PathToHandle.end()) {
            return s_PathToHandle[pathStr];
        }

        AssetHandle handle = s_NextHandle++;
        s_HandleToPath[handle] = path;
        s_PathToHandle[pathStr] = handle;
        return handle;
    }

    std::filesystem::path AssetRegistry::GetPath(AssetHandle handle) {
        if (s_HandleToPath.find(handle) != s_HandleToPath.end()) {
            return s_HandleToPath[handle];
        }
        return "";
    }

    bool AssetRegistry::Exists(AssetHandle handle) {
        return s_HandleToPath.find(handle) != s_HandleToPath.end();
    }

    bool AssetRegistry::Exists(const std::filesystem::path& path) {
        return s_PathToHandle.find(path.string()) != s_PathToHandle.end();
    }

    void AssetRegistry::Save(const std::filesystem::path& path) {
        std::ofstream fout(path);
        if (!fout.is_open()) return;

        for (auto const& [handle, p] : s_HandleToPath) {
            fout << handle << ": " << p.string() << "\n";
        }
        fout.close();
    }

    void AssetRegistry::Load(const std::filesystem::path& path) {
        std::ifstream fin(path);
        if (!fin.is_open()) return;

        s_HandleToPath.clear();
        s_PathToHandle.clear();

        std::string line;
        while (std::getline(fin, line)) {
            size_t colonPos = line.find(": ");
            if (colonPos != std::string::npos) {
                AssetHandle handle = std::stoull(line.substr(0, colonPos));
                std::filesystem::path p = line.substr(colonPos + 2);
                s_HandleToPath[handle] = p;
                s_PathToHandle[p.string()] = handle;
                if (handle >= s_NextHandle) s_NextHandle = handle + 1;
            }
        }
        fin.close();
    }

} // namespace TE

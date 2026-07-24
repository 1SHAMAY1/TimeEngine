#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Core.h"
#include <filesystem>
#include <memory>
#include <string>

namespace TE
{

struct EditorTab
{
    std::string Title;
    std::filesystem::path AssetPath;
    std::string Type;
    std::shared_ptr<Asset> LoadedAsset;
};

class TE_API AssetEditor
{
public:
    virtual ~AssetEditor() = default;
    virtual std::string GetAssetType() const = 0;
    virtual void DrawEditor(EditorTab &tab) = 0;
};

} // namespace TE

#pragma once
#include "Core/KeyCodes.hpp"
#include "Utility/MathUtils.hpp"
#include <memory>
#include <string>
#include <filesystem>

namespace TE
{

using AssetHandle = uint64_t;

class Asset
{
public:
    virtual ~Asset() = default;

    virtual AssetHandle GetHandle() const = 0;
    virtual const std::string &GetType() const = 0;
    virtual const std::string &GetName() const = 0;
    virtual const std::string &GetHoverDescription() const = 0;

    virtual std::shared_ptr<class Texture> GetIcon() const { return nullptr; }
    virtual std::shared_ptr<class Texture> GetThumbnail() const { return GetIcon(); }

    // Metadata Overrides for Modular Registration
    virtual std::string GetDefaultExtension() const { return ""; }
    virtual std::string GetDefaultIconPath() const { return "Resources/Editor/FileIcon.png"; }
    virtual TEVector2 GetDefaultIconSize() const { return {64.0f, 64.0f}; }

    virtual void SetIcon(const std::string &path, const TEVector2 &size = {64.0f, 64.0f}, const std::string &extension = "")
    {
        // This now just acts as an interface helper if needed,
        // but the metadata is primarily retrieved via virtual overrides.
    }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) {}
};

} // namespace TE

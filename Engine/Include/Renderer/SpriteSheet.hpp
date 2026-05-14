#pragma once
#include "Core/Asset/Asset.hpp"

namespace TE
{

class SpriteSheet : public Asset
{
public:
    SpriteSheet() = default;
    virtual ~SpriteSheet() = default;

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string &GetType() const override
    {
        static std::string type = "SpriteSheet";
        return type;
    }
    virtual const std::string &GetName() const override { return m_Name; }
    virtual const std::string &GetHoverDescription() const override
    {
        static std::string desc = "Sprite Sheet Asset";
        return desc;
    }

    virtual std::string GetDefaultExtension() const override { return ".tesheet"; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/SpriteSheetIcon.png"; }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) override {}

private:
    AssetHandle m_Handle = 0;
    std::string m_Name = "New Sprite Sheet";
};

} // namespace TE

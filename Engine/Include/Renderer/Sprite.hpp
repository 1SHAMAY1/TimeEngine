#pragma once
#include "Core/Asset/Asset.hpp"

namespace TE
{

class Sprite : public Asset
{
public:
    Sprite() = default;
    virtual ~Sprite() = default;

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string &GetType() const override
    {
        static std::string type = "Sprite";
        return type;
    }
    virtual const std::string &GetName() const override { return m_Name; }
    virtual const std::string &GetHoverDescription() const override
    {
        static std::string desc = "Sprite Asset";
        return desc;
    }

    virtual std::string GetDefaultExtension() const override { return ".tesprite"; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/SpriteIcon.png"; }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) override {}

private:
    AssetHandle m_Handle = 0;
    std::string m_Name = "New Sprite";
};

} // namespace TE

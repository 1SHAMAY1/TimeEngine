#pragma once
#include "Core/Asset/Asset.hpp"
#include <string>

namespace TE
{

class Texture : public Asset
{
public:
    Texture(const std::string &path);
    virtual ~Texture();

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    uint32_t GetRendererID() const { return m_RendererID; }

    // Asset interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string& GetType() const override { static std::string type = "Texture2D"; return type; }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/TextureIcon.png"; }
    virtual const std::string& GetName() const override { return m_Name; }
    virtual const std::string& GetHoverDescription() const override { return m_FilePath; }
    
    virtual std::shared_ptr<class Texture> GetIcon() const override { return nullptr; } // Will be handled by AssetManager
    virtual std::shared_ptr<class Texture> GetThumbnail() const override { return nullptr; }

    virtual void OnContentBrowserCreate(const std::filesystem::path& path) override {}

private:
    uint32_t m_RendererID;
    std::string m_FilePath;
    std::string m_Name;
    AssetHandle m_Handle;
};

} // namespace TE

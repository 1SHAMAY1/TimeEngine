#pragma once
#include "Core/Asset/Asset.hpp"
#include <string>

namespace TE
{

enum class TextureFilterMode
{
    Linear = 0,
    Nearest = 1
};

enum class TextureWrapMode
{
    Repeat = 0,
    ClampToEdge = 1,
    MirroredRepeat = 2
};

class Texture : public Asset
{
public:
    Texture(const std::string &path);
    virtual ~Texture();

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    uint32_t GetRendererID() const { return m_RendererID; }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetChannels() const { return m_Channels; }
    const std::string &GetFilePath() const { return m_FilePath; }

    TextureFilterMode GetFilterMode() const { return m_FilterMode; }
    void SetFilterMode(TextureFilterMode mode);

    TextureWrapMode GetWrapMode() const { return m_WrapMode; }
    void SetWrapMode(TextureWrapMode mode);

    bool GetGenerateMipmaps() const { return m_GenerateMipmaps; }
    void SetGenerateMipmaps(bool generate);

    bool GetPremultipliedAlpha() const { return m_PremultipliedAlpha; }
    void SetPremultipliedAlpha(bool enable) { m_PremultipliedAlpha = enable; }

    bool LoadImageSource(const std::string &path);
    void UpdateGPUParameters();

    // Asset interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const std::string &GetType() const override
    {
        static std::string type = "Texture2D";
        return type;
    }
    virtual std::string GetDefaultIconPath() const override { return "Resources/Editor/TextureIcon.png"; }
    virtual const std::string &GetName() const override { return m_Name; }
    virtual const std::string &GetHoverDescription() const override { return m_FilePath; }

    virtual std::shared_ptr<class Texture> GetIcon() const override
    {
        return nullptr;
    } // Will be handled by AssetManager
    virtual std::shared_ptr<class Texture> GetThumbnail() const override { return nullptr; }

    virtual void OnContentBrowserCreate(const std::filesystem::path &path) override;

    void SetName(const std::string &name) { m_Name = name; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint32_t m_Channels = 0;
    TextureFilterMode m_FilterMode = TextureFilterMode::Linear;
    TextureWrapMode m_WrapMode = TextureWrapMode::Repeat;
    bool m_GenerateMipmaps = false;
    bool m_PremultipliedAlpha = false;

    std::string m_FilePath;
    std::string m_Name;
    AssetHandle m_Handle;
    void *m_DX11SRV = nullptr;
    void *m_DX11Texture = nullptr;
};

} // namespace TE

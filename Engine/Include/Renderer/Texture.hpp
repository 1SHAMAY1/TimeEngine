#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"

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

class TE_API Texture : public Asset
{
public:
    Texture() = default;
    Texture(const TEString &path);
    virtual ~Texture();

    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

    uint32_t GetRendererID() const { return m_RendererID; }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetChannels() const { return m_Channels; }
    const TEString &GetFilePath() const { return m_FilePath; }

    TextureFilterMode GetFilterMode() const { return m_FilterMode; }
    void SetFilterMode(TextureFilterMode mode);

    TextureWrapMode GetWrapMode() const { return m_WrapMode; }
    void SetWrapMode(TextureWrapMode mode);

    bool GetGenerateMipmaps() const { return m_GenerateMipmaps; }
    void SetGenerateMipmaps(bool generate);

    bool GetPremultipliedAlpha() const { return m_PremultipliedAlpha; }
    void SetPremultipliedAlpha(bool enable) { m_PremultipliedAlpha = enable; }

    bool LoadImageSource(const TEString &path);
    void UpdateGPUParameters();

    // Asset interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString type = "Texture2D";
        return type;
    }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/TextureIcon.png"; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_FilePath; }

    virtual TERef<class Texture> GetIcon() const override { return nullptr; } // Will be handled by AssetManager
    virtual TERef<class Texture> GetThumbnail() const override { return nullptr; }

    virtual TERef<Asset> Clone() const override { return CreateRef<Texture>(); }
    virtual bool LoadFromFile(const TEString &path) override;

    virtual void OnContentBrowserCreate(const TEString &path) override;

    void SetName(const TEString &name) { m_Name = name; }

private:
    uint32_t m_RendererID = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    uint32_t m_Channels = 0;
    TextureFilterMode m_FilterMode = TextureFilterMode::Linear;
    TextureWrapMode m_WrapMode = TextureWrapMode::Repeat;
    bool m_GenerateMipmaps = false;
    bool m_PremultipliedAlpha = false;

    TEString m_FilePath;
    TEString m_Name;
    AssetHandle m_Handle;
    void *m_DX11SRV = nullptr;
    void *m_DX11Texture = nullptr;
};

using Texture2D = Texture;

#include "Renderer/Texture.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Renderer/RendererContext.hpp"
#include "Renderer/TextureSerializer.hpp"
#include "Utils/TEFileSystem.hpp"
#include <glad/glad.h>

#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include <d3d11.h>
#endif

TE_REGISTER_ASSET(Texture)

bool Texture::LoadFromFile(const TEString &path)
{
    TEString ext = path.GetExtension();
    if (ext == ".tetexture")
    {
        auto self = TERef<Texture>(this, [](Texture *) {});
        TextureSerializer serializer(self);
        return serializer.Deserialize(path);
    }
    else if (ext == ".png" || ext == ".jpg")
    {
        m_FilePath = path;
        m_Handle = AssetRegistry::RegisterPath(path);
        m_Name = path.GetStem();
        ImageData img = AssetManager::ImportImage(path, 4);
        if (!img.IsValid())
            return false;
        m_Width = img.Width;
        m_Height = img.Height;
        m_Channels = img.Channels;
        return true;
    }
    return false;
}

Texture::Texture(const TEString &path) : m_FilePath(path), m_RendererID(0), m_DX11SRV(nullptr), m_DX11Texture(nullptr)
{
    m_Handle = AssetRegistry::RegisterPath(path);
    m_Name = path.GetStem();

    if (path.GetExtension() == ".tetexture")
    {
        // Deserialization will load image source and GPU params via TextureSerializer
    }
    else
    {
        LoadImageSource(path);
    }
}

bool Texture::LoadImageSource(const TEString &path)
{
    if (path.empty())
        return false;

    if (m_RendererID != 0)
    {
        glDeleteTextures(1, &m_RendererID);
        m_RendererID = 0;
    }

    m_FilePath = path;

    ImageData img = AssetManager::ImportImage(path, 4);
    if (img.IsValid())
    {
        m_Width = img.Width;
        m_Height = img.Height;
        m_Channels = img.Channels;

#ifdef TE_SUPPORT_DIRECTX11
        if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
        {
            DX11Context &ctx = DX11Context::Get();
            if (!ctx.Device)
                return false;

            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = img.Width;
            desc.Height = img.Height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA subData = {};
            subData.pSysMem = img.Data();
            subData.SysMemPitch = img.Width * 4;

            ID3D11Texture2D *dxTex = nullptr;
            HRESULT hr = ctx.Device->CreateTexture2D(&desc, &subData, &dxTex);
            if (SUCCEEDED(hr))
            {
                m_DX11Texture = dxTex;
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = desc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
                srvDesc.Texture2D.MostDetailedMip = 0;

                ID3D11ShaderResourceView *dxSRV = nullptr;
                hr = ctx.Device->CreateShaderResourceView(dxTex, &srvDesc, &dxSRV);
                if (SUCCEEDED(hr))
                {
                    m_DX11SRV = dxSRV;
                }
            }
        }
        else
#endif
        {
            GLenum internalFormat = 0, dataFormat = 0;
            if (img.Channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (img.Channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }
            else if (img.Channels == 2)
            {
                internalFormat = GL_RG8;
                dataFormat = GL_RG;
            }
            else if (img.Channels == 1)
            {
                internalFormat = GL_R8;
                dataFormat = GL_RED;
            }

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, internalFormat, img.Width, img.Height);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(m_RendererID, 0, 0, 0, img.Width, img.Height, dataFormat, GL_UNSIGNED_BYTE, img.Data());
        }

        UpdateGPUParameters();
        return true;
    }
    else
    {
        TE_CORE_ERROR("Failed to load texture: {0}", path);
        return false;
    }
}

void Texture::SetFilterMode(TextureFilterMode mode)
{
    m_FilterMode = mode;
    UpdateGPUParameters();
}

void Texture::SetWrapMode(TextureWrapMode mode)
{
    m_WrapMode = mode;
    UpdateGPUParameters();
}

void Texture::SetGenerateMipmaps(bool generate)
{
    m_GenerateMipmaps = generate;
    UpdateGPUParameters();
}

void Texture::UpdateGPUParameters()
{
    if (m_RendererID == 0)
        return;

#ifdef TE_SUPPORT_DIRECTX11
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        // DirectX11 sampler state updates handled via DX11 context
        return;
    }
#endif

    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;

    if (m_FilterMode == TextureFilterMode::Nearest)
    {
        minFilter = m_GenerateMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        magFilter = GL_NEAREST;
    }
    else
    {
        minFilter = m_GenerateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        magFilter = GL_LINEAR;
    }

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, minFilter);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, magFilter);

    GLenum wrap = GL_REPEAT;
    if (m_WrapMode == TextureWrapMode::ClampToEdge)
        wrap = GL_CLAMP_TO_EDGE;
    else if (m_WrapMode == TextureWrapMode::MirroredRepeat)
        wrap = GL_MIRRORED_REPEAT;

    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);

    if (m_GenerateMipmaps)
    {
        glGenerateTextureMipmap(m_RendererID);
    }
}

Texture::~Texture()
{
    if (m_RendererID)
    {
        glDeleteTextures(1, &m_RendererID);
    }
#ifdef TE_SUPPORT_DIRECTX11
    if (m_DX11SRV)
    {
        ((ID3D11ShaderResourceView *)m_DX11SRV)->Release();
    }
    if (m_DX11Texture)
    {
        ((ID3D11Texture2D *)m_DX11Texture)->Release();
    }
#endif
}

void Texture::Bind(uint32_t slot) const
{
#ifdef TE_SUPPORT_DIRECTX11
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        DX11Context &ctx = DX11Context::Get();
        if (ctx.DeviceContext && m_DX11SRV)
        {
            ctx.DeviceContext->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView **)&m_DX11SRV);
        }
    }
    else
#endif
    {
        glBindTextureUnit(slot, m_RendererID);
    }
}

void Texture::Unbind() const
{
#ifdef TE_SUPPORT_DIRECTX11
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        DX11Context &ctx = DX11Context::Get();
        if (ctx.DeviceContext)
        {
            ID3D11ShaderResourceView *nullSRV = nullptr;
            ctx.DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
        }
    }
    else
#endif
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::OnContentBrowserCreate(const TEString &path)
{
    TEFileSystem::CreateDirectories(path);
    TEString baseName = "NewTexture";
    TEString finalPath = path / (baseName + ".tetexture");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tetexture");
    }

    TEString texName = finalPath.GetStem();
    TEString pngPath = path / (texName + ".png");

    // Write a 1x1 white PNG
    uint32_t whitePixel = 0xFFFFFFFF;
    if (!AssetManager::ExportImagePNG(pngPath, 1, 1, 4, &whitePixel))
    {
        TE_CORE_ERROR("Failed to write blank PNG for Texture at {0}", pngPath.c_str());
    }

    // Write the metadata file
    auto newTexture = CreateRef<Texture>(pngPath);
    newTexture->m_Name = texName;
    TextureSerializer serializer(newTexture);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Texture at {0}", finalPath.c_str());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Texture metadata at {0}", finalPath.c_str());
    }
}

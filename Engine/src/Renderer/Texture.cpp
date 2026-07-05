#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
// glad must come after windows.h on Windows
#include <glad/glad.h>
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include "Core/Log.h"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Renderer/RendererContext.hpp"
#include <filesystem>

namespace TE
{

Texture::Texture(const std::string &path) : m_FilePath(path), m_RendererID(0), m_DX11SRV(nullptr), m_DX11Texture(nullptr)
{
    m_Handle = AssetRegistry::RegisterPath(path);
    m_Name = std::filesystem::path(path).filename().string();
    
    ImageData img = AssetManager::ImportImage(path, (RendererContext::GetAPI() == GraphicsAPI::DirectX11) ? 4 : 0);

    if (img.Data)
    {
        if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
        {
            DX11Context& ctx = DX11Context::Get();
            if (ctx.Device)
            {
                D3D11_TEXTURE2D_DESC desc = {};
                desc.Width = img.Width;
                desc.Height = img.Height;
                desc.MipLevels = 1;
                desc.ArraySize = 1;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = 0;

                D3D11_SUBRESOURCE_DATA initData = {};
                initData.pSysMem = img.Data;
                initData.SysMemPitch = img.Width * 4;

                ID3D11Texture2D* dxTex = nullptr;
                HRESULT hr = ctx.Device->CreateTexture2D(&desc, &initData, &dxTex);
                if (SUCCEEDED(hr))
                {
                    m_DX11Texture = dxTex;
                    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                    srvDesc.Format = desc.Format;
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = 1;
                    srvDesc.Texture2D.MostDetailedMip = 0;

                    ID3D11ShaderResourceView* dxSRV = nullptr;
                    hr = ctx.Device->CreateShaderResourceView(dxTex, &srvDesc, &dxSRV);
                    if (SUCCEEDED(hr))
                    {
                        m_DX11SRV = dxSRV;
                    }
                }
            }
            AssetManager::FreeImage(img.Data);
        }
        else
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

            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(m_RendererID, 0, 0, 0, img.Width, img.Height, dataFormat, GL_UNSIGNED_BYTE, img.Data);

            AssetManager::FreeImage(img.Data);
        }
    }
    else
    {
        TE_CORE_ERROR("Failed to load texture: {0}", path);
    }
}

Texture::~Texture()
{
    if (m_RendererID)
    {
        glDeleteTextures(1, &m_RendererID);
    }
    if (m_DX11SRV)
    {
        ((ID3D11ShaderResourceView*)m_DX11SRV)->Release();
    }
    if (m_DX11Texture)
    {
        ((ID3D11Texture2D*)m_DX11Texture)->Release();
    }
}

void Texture::Bind(uint32_t slot) const
{
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        DX11Context& ctx = DX11Context::Get();
        if (ctx.DeviceContext && m_DX11SRV)
        {
            ctx.DeviceContext->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&m_DX11SRV);
        }
    }
    else
    {
        glBindTextureUnit(slot, m_RendererID);
    }
}

void Texture::Unbind() const
{
    if (RendererContext::GetAPI() == GraphicsAPI::DirectX11)
    {
        DX11Context& ctx = DX11Context::Get();
        if (ctx.DeviceContext)
        {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            ctx.DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::OnContentBrowserCreate(const std::filesystem::path &path)
{
    std::filesystem::create_directories(path);
    std::string baseName = "NewTexture";
    std::filesystem::path finalPath = path / (baseName + ".tetexture");
    int counter = 1;
    while (std::filesystem::exists(finalPath))
    {
        finalPath = path / (baseName + "_" + std::to_string(counter++) + ".tetexture");
    }

    std::string texName = finalPath.stem().string();
    std::filesystem::path pngPath = path / (texName + ".png");

    // Write a 1x1 white PNG
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    if (!AssetManager::ExportImagePNG(pngPath.string(), 1, 1, 4, whitePixel))
    {
        TE_CORE_ERROR("Failed to write blank PNG for Texture at {0}", pngPath.string());
    }

    // Write the metadata file
    auto newTexture = std::make_shared<Texture>(pngPath.string());
    newTexture->m_Name = texName;
    TextureSerializer serializer(newTexture);
    if (serializer.Serialize(finalPath))
    {
        TE_CORE_INFO("Created New Texture at {0}", finalPath.string());
    }
    else
    {
        TE_CORE_ERROR("Failed to serialize and create Texture metadata at {0}", finalPath.string());
    }
}

} // namespace TE

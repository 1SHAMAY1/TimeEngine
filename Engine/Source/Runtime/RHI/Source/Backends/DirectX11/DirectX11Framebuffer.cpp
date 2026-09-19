#include "PreRequisites.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>

#include "Renderer/DirectX11/DirectX11Framebuffer.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Log.h"

DirectX11Framebuffer::DirectX11Framebuffer(const FramebufferSpecification &spec) : m_Specification(spec)
{
    Invalidate();
}

DirectX11Framebuffer::~DirectX11Framebuffer() { Release(); }

void DirectX11Framebuffer::Release()
{
    if (m_DepthStencilView)
    {
        m_DepthStencilView->Release();
        m_DepthStencilView = nullptr;
    }
    if (m_DepthStencilTexture)
    {
        m_DepthStencilTexture->Release();
        m_DepthStencilTexture = nullptr;
    }
    if (m_ShaderResourceView)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }
    if (m_RenderTargetView)
    {
        m_RenderTargetView->Release();
        m_RenderTargetView = nullptr;
    }
    if (m_ColorTexture)
    {
        m_ColorTexture->Release();
        m_ColorTexture = nullptr;
    }
}

void DirectX11Framebuffer::Invalidate()
{
    Release();

    if (m_Specification.Width == 0 || m_Specification.Height == 0)
        return;

    ID3D11Device *device = DX11Context::Get().Device;
    if (!device)
    {
        TE_CORE_WARN("[DirectX11Framebuffer] Device not initialized when creating framebuffer.");
        return;
    }

    // 1. Color Texture & Render Target View & Shader Resource View
    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = m_Specification.Width;
    texDesc.Height = m_Specification.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_ColorTexture);
    if (FAILED(hr))
    {
        TE_CORE_ERROR("[DirectX11Framebuffer] Failed to create color texture! HRESULT: 0x{0:x}", (unsigned int)hr);
        return;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateRenderTargetView(m_ColorTexture, &rtvDesc, &m_RenderTargetView);
    if (FAILED(hr))
    {
        TE_CORE_ERROR("[DirectX11Framebuffer] Failed to create RenderTargetView! HRESULT: 0x{0:x}", (unsigned int)hr);
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_ColorTexture, &srvDesc, &m_ShaderResourceView);
    if (FAILED(hr))
    {
        TE_CORE_ERROR("[DirectX11Framebuffer] Failed to create ShaderResourceView! HRESULT: 0x{0:x}", (unsigned int)hr);
        return;
    }

    // 2. Depth Stencil Texture & Depth Stencil View
    D3D11_TEXTURE2D_DESC depthTexDesc{};
    depthTexDesc.Width = m_Specification.Width;
    depthTexDesc.Height = m_Specification.Height;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthTexDesc, nullptr, &m_DepthStencilTexture);
    if (FAILED(hr))
    {
        TE_CORE_ERROR("[DirectX11Framebuffer] Failed to create depth texture! HRESULT: 0x{0:x}", (unsigned int)hr);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(m_DepthStencilTexture, &dsvDesc, &m_DepthStencilView);
    if (FAILED(hr))
    {
        TE_CORE_ERROR("[DirectX11Framebuffer] Failed to create DepthStencilView! HRESULT: 0x{0:x}", (unsigned int)hr);
        return;
    }
}

void DirectX11Framebuffer::Bind()
{
    ID3D11DeviceContext *context = DX11Context::Get().DeviceContext;
    if (context && m_RenderTargetView)
    {
        context->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

        D3D11_VIEWPORT vp{};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = static_cast<float>(m_Specification.Width);
        vp.Height = static_cast<float>(m_Specification.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context->RSSetViewports(1, &vp);
    }
}

void DirectX11Framebuffer::Unbind()
{
    DX11Context &ctx = DX11Context::Get();
    if (ctx.DeviceContext)
    {
        if (ctx.RenderTargetView)
        {
            ctx.DeviceContext->OMSetRenderTargets(1, &ctx.RenderTargetView, ctx.DepthStencilView);
        }
        else
        {
            ID3D11RenderTargetView *nullRTV = nullptr;
            ctx.DeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
        }
    }
}

void DirectX11Framebuffer::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0 || (m_Specification.Width == width && m_Specification.Height == height))
        return;

    m_Specification.Width = width;
    m_Specification.Height = height;
    Invalidate();
}

uintptr_t DirectX11Framebuffer::GetColorAttachmentRendererID() const
{
    return reinterpret_cast<uintptr_t>(m_ShaderResourceView);
}

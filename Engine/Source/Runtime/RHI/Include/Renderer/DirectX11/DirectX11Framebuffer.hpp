#pragma once
#include "Framebuffer.hpp"

// Forward-declare D3D11 pointers to avoid pulling d3d11.h into headers
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

class DirectX11Framebuffer : public Framebuffer
{
public:
    DirectX11Framebuffer(const FramebufferSpecification &spec);
    virtual ~DirectX11Framebuffer() override;

    void Invalidate();
    void Release();

    virtual void Bind() override;
    virtual void Unbind() override;
    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual uintptr_t GetColorAttachmentRendererID() const override;
    virtual const FramebufferSpecification &GetSpecification() const override { return m_Specification; }

    ID3D11RenderTargetView *GetRenderTargetView() const { return m_RenderTargetView; }
    ID3D11DepthStencilView *GetDepthStencilView() const { return m_DepthStencilView; }
    ID3D11ShaderResourceView *GetShaderResourceView() const { return m_ShaderResourceView; }

private:
    FramebufferSpecification m_Specification;
    ID3D11Texture2D *m_ColorTexture = nullptr;
    ID3D11RenderTargetView *m_RenderTargetView = nullptr;
    ID3D11ShaderResourceView *m_ShaderResourceView = nullptr;
    ID3D11Texture2D *m_DepthStencilTexture = nullptr;
    ID3D11DepthStencilView *m_DepthStencilView = nullptr;
};

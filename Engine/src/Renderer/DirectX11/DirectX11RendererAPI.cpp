// Windows headers MUST come first to avoid macro pollution when other headers are included
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Engine headers after Windows/D3D11
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Renderer/OpenGL/OpenGLRendererAPI.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESRendererAPI.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#include "Renderer/Vulkan/VulkanRendererAPI.hpp"

#include <cstring>
#include <iostream>
#include <string>

namespace TE
{

// -------------------------------------------------------------------------
//  File-local helper: BlendFactor -> D3D11_BLEND
//  Kept here (not in the header) because D3D11_BLEND requires d3d11.h
// -------------------------------------------------------------------------
static D3D11_BLEND ToDXBlend(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:
        return D3D11_BLEND_ZERO;
    case BlendFactor::One:
        return D3D11_BLEND_ONE;
    case BlendFactor::SrcAlpha:
        return D3D11_BLEND_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case BlendFactor::DstColor:
        return D3D11_BLEND_DEST_COLOR;
    }
    return D3D11_BLEND_ONE;
}

// -------------------------------------------------------------------------
//  RendererAPI factory — consolidated here so all backend headers are visible
// -------------------------------------------------------------------------
GraphicsAPI RendererAPI::GetAPI() { return RendererContext::GetAPI(); }

std::unique_ptr<RendererAPI> RendererAPI::Create()
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
    case GraphicsAPI::OpenGL:
        return std::make_unique<OpenGLRendererAPI>();
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return std::make_unique<OpenGLESRendererAPI>();
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
        return std::make_unique<VulkanRendererAPI>();
    case GraphicsAPI::DirectX11:
        return std::make_unique<DirectX11RendererAPI>();
    }
    return nullptr;
}

// -------------------------------------------------------------------------
//  Lifecycle
// -------------------------------------------------------------------------
DirectX11RendererAPI::~DirectX11RendererAPI()
{
    ReleaseRenderTargets();
    DX11Context &ctx = DX11Context::Get();
    if (ctx.SwapChain)
    {
        ctx.SwapChain->Release();
        ctx.SwapChain = nullptr;
    }
    if (ctx.DeviceContext)
    {
        ctx.DeviceContext->Release();
        ctx.DeviceContext = nullptr;
    }
    if (ctx.Device)
    {
        ctx.Device->Release();
        ctx.Device = nullptr;
    }
}

void DirectX11RendererAPI::Init()
{
    // No-op: InitWithWindow() is called from WindowsWindow after HWND is available.
}

void DirectX11RendererAPI::InitWithWindow(void *hwnd, uint32_t width, uint32_t height)
{
    DX11Context &ctx = DX11Context::Get();
    m_ViewportW = width;
    m_ViewportH = height;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = static_cast<HWND>(hwnd);
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

    UINT createFlags = 0;
#if defined(_DEBUG) || defined(TE_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, featureLevels,
                                               ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &scd, &ctx.SwapChain,
                                               &ctx.Device, &featureLevel, &ctx.DeviceContext);

    if (FAILED(hr))
    {
        std::cerr << "[DirectX11] D3D11CreateDeviceAndSwapChain failed: 0x" << std::hex << hr << std::endl;
        return;
    }

    // Retrieve GPU description via DXGI
    IDXGIDevice *dxgiDevice = nullptr;
    IDXGIAdapter *dxgiAdapter = nullptr;
    if (SUCCEEDED(ctx.Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice))))
    {
        if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter)))
        {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(dxgiAdapter->GetDesc(&desc)))
            {
                char buf[256] = {};
                WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, buf, 256, nullptr, nullptr);
                m_GPUDescription = buf;
            }
            dxgiAdapter->Release();
        }
        dxgiDevice->Release();
    }

    CreateRenderTargetView();
    CreateDepthStencilView(width, height);
    SetViewport(0, 0, width, height);

    // Default alpha blending state
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState *blendState = nullptr;
    ctx.Device->CreateBlendState(&blendDesc, &blendState);
    ctx.DeviceContext->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
    blendState->Release();
}

// -------------------------------------------------------------------------
//  Private helpers
// -------------------------------------------------------------------------
void DirectX11RendererAPI::CreateRenderTargetView()
{
    DX11Context &ctx = DX11Context::Get();
    ID3D11Texture2D *backBuffer = nullptr;
    ctx.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer));
    ctx.Device->CreateRenderTargetView(backBuffer, nullptr, &ctx.RenderTargetView);
    backBuffer->Release();
    ctx.DeviceContext->OMSetRenderTargets(1, &ctx.RenderTargetView, ctx.DepthStencilView);
}

void DirectX11RendererAPI::CreateDepthStencilView(uint32_t width, uint32_t height)
{
    DX11Context &ctx = DX11Context::Get();

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ctx.Device->CreateTexture2D(&depthDesc, nullptr, &ctx.DepthStencilTex);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ctx.Device->CreateDepthStencilView(ctx.DepthStencilTex, &dsvDesc, &ctx.DepthStencilView);

    ctx.DeviceContext->OMSetRenderTargets(1, &ctx.RenderTargetView, ctx.DepthStencilView);
}

void DirectX11RendererAPI::ReleaseRenderTargets()
{
    DX11Context &ctx = DX11Context::Get();
    if (ctx.DepthStencilView)
    {
        ctx.DepthStencilView->Release();
        ctx.DepthStencilView = nullptr;
    }
    if (ctx.DepthStencilTex)
    {
        ctx.DepthStencilTex->Release();
        ctx.DepthStencilTex = nullptr;
    }
    if (ctx.RenderTargetView)
    {
        ctx.RenderTargetView->Release();
        ctx.RenderTargetView = nullptr;
    }
}

// -------------------------------------------------------------------------
//  RendererAPI interface implementation
// -------------------------------------------------------------------------
void DirectX11RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    m_ViewportX = static_cast<int>(x);
    m_ViewportY = static_cast<int>(y);
    m_ViewportW = width;
    m_ViewportH = height;

    if (!DX11Context::Get().DeviceContext)
        return;

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = static_cast<float>(x);
    vp.TopLeftY = static_cast<float>(y);
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    DX11Context::Get().DeviceContext->RSSetViewports(1, &vp);
}

void DirectX11RendererAPI::SetClearColor(const glm::vec4 &color) { m_ClearColor = color; }

void DirectX11RendererAPI::Clear()
{
    DX11Context &ctx = DX11Context::Get();
    float c[4] = {m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a};
    if (ctx.RenderTargetView)
        ctx.DeviceContext->ClearRenderTargetView(ctx.RenderTargetView, c);
    if (ctx.DepthStencilView)
        ctx.DeviceContext->ClearDepthStencilView(ctx.DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,
                                                 0);
}

void DirectX11RendererAPI::DrawIndexed(uint32_t /*vao*/, uint32_t indexCount)
{
    DX11Context::Get().DeviceContext->DrawIndexed(indexCount, 0, 0);
}

void DirectX11RendererAPI::SetBlendMode(int blendMode)
{
    DX11Context &ctx = DX11Context::Get();
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if (blendMode == 1)
    { // Additive
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    }
    else if (blendMode == 2)
    { // Multiplicative
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    }
    else
    { // Normal alpha
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    }

    ID3D11BlendState *bs = nullptr;
    ctx.Device->CreateBlendState(&blendDesc, &bs);
    ctx.DeviceContext->OMSetBlendState(bs, nullptr, 0xFFFFFFFF);
    bs->Release();
}

bool DirectX11RendererAPI::LoadLoader(void *(* /*loadProc*/)(const char *))
{
    return true; // D3D11 is statically linked, no loader needed.
}

std::string DirectX11RendererAPI::GetVersionString() { return "Direct3D 11.0"; }
std::string DirectX11RendererAPI::GetGPUVendor() { return m_GPUDescription.empty() ? "Unknown" : m_GPUDescription; }
std::string DirectX11RendererAPI::GetGPURenderer()
{
    return m_GPUDescription.empty() ? "Direct3D 11 Hardware" : m_GPUDescription;
}

void DirectX11RendererAPI::GetViewport(int *viewport)
{
    if (!viewport)
        return;
    viewport[0] = m_ViewportX;
    viewport[1] = m_ViewportY;
    viewport[2] = static_cast<int>(m_ViewportW);
    viewport[3] = static_cast<int>(m_ViewportH);
}

void DirectX11RendererAPI::GetClearColor(float *color)
{
    if (!color)
        return;
    color[0] = m_ClearColor.r;
    color[1] = m_ClearColor.g;
    color[2] = m_ClearColor.b;
    color[3] = m_ClearColor.a;
}

void DirectX11RendererAPI::ReadPixelsRGBA(int x, int y, int width, int height, void *outPixels)
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.Device || !ctx.DeviceContext || !ctx.SwapChain)
        return;

    ID3D11Texture2D *backBuffer = nullptr;
    if (FAILED(ctx.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer))))
        return;

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ID3D11Texture2D *stagingTex = nullptr;
    if (FAILED(ctx.Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTex)))
    {
        backBuffer->Release();
        return;
    }

    ctx.DeviceContext->CopyResource(stagingTex, backBuffer);
    backBuffer->Release();

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx.DeviceContext->Map(stagingTex, 0, D3D11_MAP_READ, 0, &mapped)))
    {
        uint8_t *dst = static_cast<uint8_t *>(outPixels);
        uint8_t *src = static_cast<uint8_t *>(mapped.pData);
        uint32_t stride = mapped.RowPitch;
        // Flip rows vertically to match OpenGL convention (y=0 is bottom-left)
        for (int row = 0; row < height; ++row)
        {
            int srcRow = static_cast<int>(desc.Height) - 1 - (y + row);
            if (srcRow < 0 || srcRow >= static_cast<int>(desc.Height))
                continue;
            std::memcpy(dst + row * width * 4, src + srcRow * stride + x * 4, width * 4);
        }
        ctx.DeviceContext->Unmap(stagingTex, 0);
    }
    stagingTex->Release();
}

void DirectX11RendererAPI::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
    DX11Context &ctx = DX11Context::Get();
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = ToDXBlend(src);
    blendDesc.RenderTarget[0].DestBlend = ToDXBlend(dst);
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11BlendState *bs = nullptr;
    ctx.Device->CreateBlendState(&blendDesc, &bs);
    ctx.DeviceContext->OMSetBlendState(bs, nullptr, 0xFFFFFFFF);
    bs->Release();
}

void DirectX11RendererAPI::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                                BlendFactor dstAlpha)
{
    DX11Context &ctx = DX11Context::Get();
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = ToDXBlend(srcRGB);
    blendDesc.RenderTarget[0].DestBlend = ToDXBlend(dstRGB);
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = ToDXBlend(srcAlpha);
    blendDesc.RenderTarget[0].DestBlendAlpha = ToDXBlend(dstAlpha);
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11BlendState *bs = nullptr;
    ctx.Device->CreateBlendState(&blendDesc, &bs);
    ctx.DeviceContext->OMSetBlendState(bs, nullptr, 0xFFFFFFFF);
    bs->Release();
}

} // namespace TE

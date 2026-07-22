#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Renderer/DirectX11/DirectX11VertexBuffer.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include <Windows.h>
#include <d3d11.h>
#include <iostream>
#include <cstring>

namespace TE
{

DirectX11VertexBuffer::DirectX11VertexBuffer(float *vertices, uint32_t size) : m_Size(size)
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.Device)
    {
        std::cerr << "[DirectX11] VertexBuffer: Device is null. Did you call InitWithWindow first?\n";
        return;
    }

    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = size;
    bufDesc.Usage = D3D11_USAGE_DYNAMIC; // allows Map/Unmap updates
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = ctx.Device->CreateBuffer(&bufDesc, vertices ? &initData : nullptr, &m_Buffer);
    if (FAILED(hr))
        std::cerr << "[DirectX11] Failed to create vertex buffer: 0x" << std::hex << hr << "\n";
}

DirectX11VertexBuffer::~DirectX11VertexBuffer()
{
    if (m_Buffer)
    {
        m_Buffer->Release();
        m_Buffer = nullptr;
    }
}

void DirectX11VertexBuffer::Bind() const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext || !m_Buffer)
        return;
    UINT offset = 0;
    ctx.DeviceContext->IASetVertexBuffers(0, 1, &m_Buffer, &m_Stride, &offset);
}

void DirectX11VertexBuffer::Unbind() const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext)
        return;
    ID3D11Buffer *nullBuf = nullptr;
    UINT stride = 0, offset = 0;
    ctx.DeviceContext->IASetVertexBuffers(0, 1, &nullBuf, &stride, &offset);
}

void DirectX11VertexBuffer::SetData(float *vertices, uint32_t size) const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext || !m_Buffer || !vertices)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = ctx.DeviceContext->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        std::memcpy(mapped.pData, vertices, size);
        ctx.DeviceContext->Unmap(m_Buffer, 0);
    }
    else
    {
        std::cerr << "[DirectX11] Failed to map vertex buffer: 0x" << std::hex << hr << "\n";
    }
}

} // namespace TE

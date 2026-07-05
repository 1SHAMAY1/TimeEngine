#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include "Renderer/DirectX11/DirectX11IndexBuffer.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include <iostream>

namespace TE {

    DirectX11IndexBuffer::DirectX11IndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count)
    {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.Device) {
            std::cerr << "[DirectX11] IndexBuffer: Device is null. Did you call InitWithWindow first?\n";
            return;
        }

        D3D11_BUFFER_DESC bufDesc = {};
        bufDesc.ByteWidth      = count * sizeof(uint32_t);
        bufDesc.Usage          = D3D11_USAGE_DYNAMIC;
        bufDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = indices;

        HRESULT hr = ctx.Device->CreateBuffer(&bufDesc, indices ? &initData : nullptr, &m_Buffer);
        if (FAILED(hr))
            std::cerr << "[DirectX11] Failed to create index buffer: 0x" << std::hex << hr << "\n";
    }

    DirectX11IndexBuffer::~DirectX11IndexBuffer() {
        if (m_Buffer) { m_Buffer->Release(); m_Buffer = nullptr; }
    }

    void DirectX11IndexBuffer::Bind() const {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.DeviceContext || !m_Buffer) return;
        ctx.DeviceContext->IASetIndexBuffer(m_Buffer, DXGI_FORMAT_R32_UINT, 0);
    }

    void DirectX11IndexBuffer::Unbind() const {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.DeviceContext) return;
        ctx.DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
    }

    void DirectX11IndexBuffer::SetData(uint32_t* indices, uint32_t count) const {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.DeviceContext || !m_Buffer || !indices) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = ctx.DeviceContext->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            std::memcpy(mapped.pData, indices, count * sizeof(uint32_t));
            ctx.DeviceContext->Unmap(m_Buffer, 0);
        } else {
            std::cerr << "[DirectX11] Failed to map index buffer: 0x" << std::hex << hr << "\n";
        }
    }

    uint32_t DirectX11IndexBuffer::GetCount() const {
        return m_Count;
    }

}

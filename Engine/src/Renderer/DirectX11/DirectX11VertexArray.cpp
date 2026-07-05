#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include "Renderer/DirectX11/DirectX11VertexArray.hpp"
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include <iostream>

namespace TE {

    DirectX11VertexArray::DirectX11VertexArray() {
        // Nothing to create here; D3D11 has no VAO object.
        // Input layout is built when AddVertexBuffer is called (requires compiled shader blob).
    }

    DirectX11VertexArray::~DirectX11VertexArray() {
        if (m_InputLayout) { m_InputLayout->Release(); m_InputLayout = nullptr; }
    }

    void DirectX11VertexArray::Bind() const {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.DeviceContext) return;

        // Set input layout (describes per-vertex attribute format to the IA stage)
        ctx.DeviceContext->IASetInputLayout(m_InputLayout);
        ctx.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Bind vertex and index buffers via their own Bind() implementations
        if (m_VertexBuffer) m_VertexBuffer->Bind();
        if (m_IndexBuffer)  m_IndexBuffer->Bind();
    }

    void DirectX11VertexArray::Unbind() const {
        DX11Context& ctx = DX11Context::Get();
        if (!ctx.DeviceContext) return;

        ctx.DeviceContext->IASetInputLayout(nullptr);

        if (m_VertexBuffer) m_VertexBuffer->Unbind();
        if (m_IndexBuffer)  m_IndexBuffer->Unbind();
    }

    void DirectX11VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
        m_VertexBuffer = vertexBuffer;
        // Note: The ID3D11InputLayout requires a compiled shader's bytecode at creation time.
        // It should be built by DirectX11Shader after shader compilation, or by the caller
        // using DX11Context::Get().Device->CreateInputLayout(descs, count, shaderBlob, ...).
        // For the standard float3 position layout used by the engine's Renderer2D batcher:
        //
        //   D3D11_INPUT_ELEMENT_DESC layout[] = {
        //       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //       { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //       { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //   };
        //
        // This is wired up per-pipeline when DirectX11Shader::Bind() is called.
    }

    void DirectX11VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
        m_IndexBuffer = indexBuffer;
        if (m_IndexBuffer) m_IndexBuffer->Bind();
    }

}

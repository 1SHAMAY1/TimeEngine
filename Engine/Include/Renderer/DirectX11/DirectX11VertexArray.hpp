#pragma once
#include "Renderer/VertexArray.hpp"

struct ID3D11InputLayout;

namespace TE
{

// In D3D11, there is no VAO equivalent. This class groups vertex + index buffers
// together and manages an ID3D11InputLayout that describes the vertex layout to the IA stage.
// The input layout is created lazily the first time Bind() is called, once a shader is active.
class DirectX11VertexArray : public VertexArray
{
public:
    DirectX11VertexArray();
    virtual ~DirectX11VertexArray();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(VertexBuffer *vertexBuffer) override;
    virtual void SetIndexBuffer(IndexBuffer *indexBuffer) override;

    // GetRendererID() returns 0 — D3D11 has no VAO IDs.
    // DrawIndexed is driven by the DeviceContext, not an integer handle.
    virtual uint32_t GetRendererID() const override { return 0; }

private:
    VertexBuffer *m_VertexBuffer = nullptr;
    IndexBuffer *m_IndexBuffer = nullptr;
    ID3D11InputLayout *m_InputLayout = nullptr;
};

} // namespace TE

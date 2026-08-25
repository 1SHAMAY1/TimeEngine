#pragma once

#include "Renderer/VertexArray.hpp"

class MetalVertexArray : public VertexArray
{
public:
    MetalVertexArray();
    virtual ~MetalVertexArray();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer) override;
    virtual void SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer) override;

    virtual uint32_t GetRendererID() const override { return 0; }

    TERef<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
    TERef<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

private:
    TERef<VertexBuffer> m_VertexBuffer = nullptr;
    TERef<IndexBuffer> m_IndexBuffer = nullptr;
};


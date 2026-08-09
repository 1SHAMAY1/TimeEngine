#pragma once

#include "Renderer/VertexArray.hpp"

namespace TE
{
class MetalVertexArray : public VertexArray
{
public:
    MetalVertexArray();
    virtual ~MetalVertexArray();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(VertexBuffer *vertexBuffer) override;
    virtual void SetIndexBuffer(IndexBuffer *indexBuffer) override;

    virtual uint32_t GetRendererID() const override { return 0; }

    VertexBuffer *GetVertexBuffer() const { return m_VertexBuffer; }
    IndexBuffer *GetIndexBuffer() const { return m_IndexBuffer; }

private:
    VertexBuffer *m_VertexBuffer = nullptr;
    IndexBuffer *m_IndexBuffer = nullptr;
};
} // namespace TE

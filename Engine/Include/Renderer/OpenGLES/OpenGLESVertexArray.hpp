#pragma once
#include "Renderer/VertexArray.hpp"

class OpenGLESVertexArray : public VertexArray
{
public:
    OpenGLESVertexArray();
    virtual ~OpenGLESVertexArray();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer) override;
    virtual void SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer) override;

    virtual uint32_t GetRendererID() const override { return m_RendererID; }

private:
    uint32_t m_RendererID;
    TERef<VertexBuffer> m_VertexBuffer = nullptr;
    TERef<IndexBuffer> m_IndexBuffer = nullptr;
};

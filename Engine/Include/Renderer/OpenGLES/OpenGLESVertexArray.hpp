#pragma once
#include "Renderer/VertexArray.hpp"

namespace TE {
    class OpenGLESVertexArray : public VertexArray {
    public:
        OpenGLESVertexArray();
        virtual ~OpenGLESVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) override;
        virtual void SetIndexBuffer(IndexBuffer* indexBuffer) override;

        virtual uint32_t GetRendererID() const override { return m_RendererID; }

    private:
        uint32_t m_RendererID;
        VertexBuffer* m_VertexBuffer = nullptr;
        IndexBuffer*  m_IndexBuffer  = nullptr;
    };
}

#pragma once
#include "Renderer/VertexArray.hpp"

namespace TE {
    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();
        virtual ~OpenGLVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) override;
        virtual void SetIndexBuffer(IndexBuffer* indexBuffer) override;
        
        virtual uint32_t GetRendererID() const override { return m_RendererID; }

    private:
        uint32_t m_RendererID;
        VertexBuffer* m_VertexBuffer;
        IndexBuffer* m_IndexBuffer;
    };
}

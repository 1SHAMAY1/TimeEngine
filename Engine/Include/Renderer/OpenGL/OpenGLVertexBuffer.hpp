#pragma once

#include "Renderer/VertexBuffer.hpp"

namespace TE {
    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(float* vertices, uint32_t size) const override;

    private:
        uint32_t m_RendererID;
   
    };
}

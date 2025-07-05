#pragma once
#include "Renderer/IndexBuffer.hpp"

namespace TE {
    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t Count);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(uint32_t* indices, uint32_t Size) const override;

        virtual uint32_t GetCount() const override;

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };
}

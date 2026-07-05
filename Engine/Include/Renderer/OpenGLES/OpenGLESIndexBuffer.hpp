#pragma once
#include "Renderer/IndexBuffer.hpp"

namespace TE {
    class OpenGLESIndexBuffer : public IndexBuffer {
    public:
        OpenGLESIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLESIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(uint32_t* indices, uint32_t size) const override;

        virtual uint32_t GetCount() const override;

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };
}

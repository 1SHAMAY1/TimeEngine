#pragma once
#include "Renderer/VertexBuffer.hpp"

struct ID3D11Buffer;

namespace TE {

    class DirectX11VertexBuffer : public VertexBuffer {
    public:
        DirectX11VertexBuffer(float* vertices, uint32_t size);
        virtual ~DirectX11VertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(float* vertices, uint32_t size) const override;

    private:
        ID3D11Buffer* m_Buffer = nullptr;
        uint32_t      m_Stride = 0;   // stride in bytes (always sizeof(float) * components)
        uint32_t      m_Size   = 0;
    };

}

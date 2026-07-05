#pragma once
#include "Renderer/IndexBuffer.hpp"

struct ID3D11Buffer;

namespace TE {

    class DirectX11IndexBuffer : public IndexBuffer {
    public:
        DirectX11IndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~DirectX11IndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(uint32_t* indices, uint32_t count) const override;
        virtual uint32_t GetCount() const override;

    private:
        ID3D11Buffer* m_Buffer = nullptr;
        uint32_t      m_Count  = 0;
    };

}

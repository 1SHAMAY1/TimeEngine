#pragma once

#include "Renderer/VertexBuffer.hpp"

class MetalVertexBuffer : public VertexBuffer
{
public:
    MetalVertexBuffer(float *vertices, uint32_t size);
    virtual ~MetalVertexBuffer();

    virtual void Bind() const override;
    virtual void Unbind() const override;
    virtual void SetData(float *vertices, uint32_t size) const override;

    void *GetBuffer() const { return m_Buffer; }

private:
    void *m_Buffer = nullptr;
    uint32_t m_Size = 0;
};


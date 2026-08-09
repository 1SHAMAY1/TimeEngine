#pragma once

#include "Renderer/IndexBuffer.hpp"

namespace TE
{
class MetalIndexBuffer : public IndexBuffer
{
public:
    MetalIndexBuffer(uint32_t *indices, uint32_t count);
    virtual ~MetalIndexBuffer();

    virtual void Bind() const override;
    virtual void Unbind() const override;
    virtual void SetData(uint32_t *indices, uint32_t count) const override;

    virtual uint32_t GetCount() const override { return m_Count; }
    void *GetBuffer() const { return m_Buffer; }

private:
    void *m_Buffer = nullptr;
    uint32_t m_Count = 0;
};
} // namespace TE

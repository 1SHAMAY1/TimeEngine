#pragma once
#include "Renderer/VertexBuffer.hpp"
#include <volk.h>

namespace TE
{
class VulkanVertexBuffer : public VertexBuffer
{
public:
    VulkanVertexBuffer(float *vertices, uint32_t size);
    virtual ~VulkanVertexBuffer() override;

    virtual void Bind() const override;
    virtual void Unbind() const override;
    virtual void SetData(float *vertices, uint32_t size) const override;

    VkBuffer GetVkBuffer() const { return m_Buffer; }

private:
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;
};
} // namespace TE

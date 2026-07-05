#pragma once
#include "Renderer/IndexBuffer.hpp"
#include <volk.h>

namespace TE {
    class VulkanIndexBuffer : public IndexBuffer {
    public:
        VulkanIndexBuffer(uint32_t* indices, uint32_t Count);
        virtual ~VulkanIndexBuffer() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(uint32_t* indices, uint32_t Size) const override;

        virtual uint32_t GetCount() const override;

        VkBuffer GetVkBuffer() const { return m_Buffer; }

    private:
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        uint32_t m_Count;
    };
}

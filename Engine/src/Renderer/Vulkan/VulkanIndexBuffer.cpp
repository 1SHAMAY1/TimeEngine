#include "Renderer/Vulkan/VulkanIndexBuffer.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include <iostream>

namespace TE
{

VulkanIndexBuffer::VulkanIndexBuffer(uint32_t *indices, uint32_t Count) : m_Count(Count)
{
    VkDevice device = VulkanContext::GetDevice();
    if (device == VK_NULL_HANDLE)
    {
        std::cout << "[Vulkan] Error: Device is null, skipping Index Buffer creation\n";
        return;
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = Count * sizeof(uint32_t);
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
    {
        std::cout << "[Vulkan] Error: Failed to create Index VkBuffer\n";
    }
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    VkDevice device = VulkanContext::GetDevice();
    if (device != VK_NULL_HANDLE && m_Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, m_Buffer, nullptr);
    }
    if (device != VK_NULL_HANDLE && m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, m_Memory, nullptr);
    }
}

void VulkanIndexBuffer::Bind() const
{
    // Bound during command buffer recording:
    // vkCmdBindIndexBuffer(commandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanIndexBuffer::Unbind() const {}

void VulkanIndexBuffer::SetData(uint32_t *indices, uint32_t Size) const
{
    VkDevice device = VulkanContext::GetDevice();
    if (device != VK_NULL_HANDLE && m_Memory != VK_NULL_HANDLE)
    {
        void *data;
        vkMapMemory(device, m_Memory, 0, Size, 0, &data);
        memcpy(data, indices, (size_t)Size);
        vkUnmapMemory(device, m_Memory);
    }
}

uint32_t VulkanIndexBuffer::GetCount() const { return m_Count; }

} // namespace TE

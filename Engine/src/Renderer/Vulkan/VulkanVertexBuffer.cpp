#include "Renderer/Vulkan/VulkanVertexBuffer.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include <iostream>

namespace TE
{

VulkanVertexBuffer::VulkanVertexBuffer(float *vertices, uint32_t size)
{
    VkDevice device = VulkanContext::GetDevice();
    if (device == VK_NULL_HANDLE)
    {
        std::cout << "[Vulkan] Error: Device is null, skipping Vertex Buffer creation\n";
        return;
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
    {
        std::cout << "[Vulkan] Error: Failed to create VkBuffer\n";
    }
}

VulkanVertexBuffer::~VulkanVertexBuffer()
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

void VulkanVertexBuffer::Bind() const
{
    // In Vulkan, binding is performed during command recording:
    // vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Buffer, offsets);
}

void VulkanVertexBuffer::Unbind() const {}

void VulkanVertexBuffer::SetData(float *vertices, uint32_t size) const
{
    VkDevice device = VulkanContext::GetDevice();
    if (device != VK_NULL_HANDLE && m_Memory != VK_NULL_HANDLE)
    {
        void *data;
        vkMapMemory(device, m_Memory, 0, size, 0, &data);
        memcpy(data, vertices, (size_t)size);
        vkUnmapMemory(device, m_Memory);
    }
}

} // namespace TE

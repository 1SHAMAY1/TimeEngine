#include "Renderer/Vulkan/VulkanVertexArray.hpp"
#include <iostream>

namespace TE
{

VulkanVertexArray::VulkanVertexArray() {}

VulkanVertexArray::~VulkanVertexArray() {}

void VulkanVertexArray::Bind() const
{
    // Vulkan binds buffers individually to command buffer
}

void VulkanVertexArray::Unbind() const {}

void VulkanVertexArray::AddVertexBuffer(VertexBuffer *vertexBuffer) { m_VertexBuffer = vertexBuffer; }

void VulkanVertexArray::SetIndexBuffer(IndexBuffer *indexBuffer) { m_IndexBuffer = indexBuffer; }

} // namespace TE

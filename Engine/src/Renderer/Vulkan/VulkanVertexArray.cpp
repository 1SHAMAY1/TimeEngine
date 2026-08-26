#include "Renderer/Vulkan/VulkanVertexArray.hpp"
#include "Core/PreRequisites.h"
#include <iostream>

VulkanVertexArray::VulkanVertexArray() {}

VulkanVertexArray::~VulkanVertexArray() {}

void VulkanVertexArray::Bind() const
{
    // Vulkan binds buffers individually to command buffer
}

void VulkanVertexArray::Unbind() const {}

void VulkanVertexArray::AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer) { m_VertexBuffer = vertexBuffer; }

void VulkanVertexArray::SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer) { m_IndexBuffer = indexBuffer; }

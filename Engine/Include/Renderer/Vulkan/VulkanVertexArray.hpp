#pragma once
#include "Renderer/VertexArray.hpp"
#include <volk.h>

namespace TE
{
class VulkanVertexArray : public VertexArray
{
public:
    VulkanVertexArray();
    virtual ~VulkanVertexArray() override;

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(VertexBuffer *vertexBuffer) override;
    virtual void SetIndexBuffer(IndexBuffer *indexBuffer) override;

    virtual uint32_t GetRendererID() const override { return 0; } // Vulkan uses custom pipelines and bindings

private:
    VertexBuffer *m_VertexBuffer = nullptr;
    IndexBuffer *m_IndexBuffer = nullptr;
};
} // namespace TE

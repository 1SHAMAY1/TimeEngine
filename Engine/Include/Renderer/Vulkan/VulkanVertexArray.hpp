#pragma once
#include "Renderer/VertexArray.hpp"
#include <volk.h>

class VulkanVertexArray : public VertexArray
{
public:
    VulkanVertexArray();
    virtual ~VulkanVertexArray() override;

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer) override;
    virtual void SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer) override;

    virtual uint32_t GetRendererID() const override { return 0; } // Vulkan uses custom pipelines and bindings

private:
    TERef<VertexBuffer> m_VertexBuffer = nullptr;
    TERef<IndexBuffer> m_IndexBuffer = nullptr;
};


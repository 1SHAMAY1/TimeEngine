#pragma once
#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"

class TE_API VertexArray
{
public:
    static TERef<VertexArray> Create();

    virtual ~VertexArray() = default;
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer) = 0;
    virtual void SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer) = 0;

    // Get the renderer ID (OpenGL VAO ID)
    virtual uint32_t GetRendererID() const = 0;
};

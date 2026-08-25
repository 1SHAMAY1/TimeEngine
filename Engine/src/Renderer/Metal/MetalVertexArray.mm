#include "Renderer/Metal/MetalVertexArray.hpp"

namespace TE
{

MetalVertexArray::MetalVertexArray()
{
}

MetalVertexArray::~MetalVertexArray()
{
}

void MetalVertexArray::Bind() const
{
}

void MetalVertexArray::Unbind() const
{
}

void MetalVertexArray::AddVertexBuffer(const TERef<VertexBuffer> &vertexBuffer)
{
    m_VertexBuffer = vertexBuffer;
}

void MetalVertexArray::SetIndexBuffer(const TERef<IndexBuffer> &indexBuffer)
{
    m_IndexBuffer = indexBuffer;
}

} // namespace TE

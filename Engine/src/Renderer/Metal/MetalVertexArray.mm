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

void MetalVertexArray::AddVertexBuffer(VertexBuffer *vertexBuffer)
{
    m_VertexBuffer = vertexBuffer;
}

void MetalVertexArray::SetIndexBuffer(IndexBuffer *indexBuffer)
{
    m_IndexBuffer = indexBuffer;
}

} // namespace TE

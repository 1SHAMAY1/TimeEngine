#include "Renderer/OpenGLES/OpenGLESVertexBuffer.hpp"
#include <glad/glad.h>

namespace TE
{

OpenGLESVertexBuffer::OpenGLESVertexBuffer(float *vertices, uint32_t size)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

OpenGLESVertexBuffer::~OpenGLESVertexBuffer() { glDeleteBuffers(1, &m_RendererID); }

void OpenGLESVertexBuffer::Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); }

void OpenGLESVertexBuffer::Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void OpenGLESVertexBuffer::SetData(float *vertices, uint32_t size) const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
}

} // namespace TE

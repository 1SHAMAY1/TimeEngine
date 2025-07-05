#include "Renderer/OpenGL/OpenGLVertexBuffer.hpp"
#include <glad/glad.h>

#include "Renderer/OpenGL/OpenGLVertexArray.hpp"

namespace TE {
    OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size) {
        glCreateBuffers(1,&m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLVertexBuffer::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLVertexBuffer::Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::SetData(float* vertices, uint32_t size) const
    {
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }
}

#include "Renderer/OpenGL/OpenGLIndexBuffer.hpp"
#include <glad/glad.h>

namespace TE {
    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t Count)
        : m_Count(Count) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLIndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        
    }

    void OpenGLIndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void OpenGLIndexBuffer::SetData(uint32_t* indices, uint32_t Size) const
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    uint32_t OpenGLIndexBuffer::GetCount() const {
        return m_Count;
    }
}

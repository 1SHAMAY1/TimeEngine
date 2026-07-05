#include "Renderer/OpenGLES/OpenGLESIndexBuffer.hpp"
#include <glad/glad.h>

namespace TE {

    OpenGLESIndexBuffer::OpenGLESIndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLESIndexBuffer::~OpenGLESIndexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLESIndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLESIndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void OpenGLESIndexBuffer::SetData(uint32_t* indices, uint32_t size) const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), indices, GL_DYNAMIC_DRAW);
    }

    uint32_t OpenGLESIndexBuffer::GetCount() const {
        return m_Count;
    }

} // namespace TE

#include "Renderer/OpenGLES/OpenGLESVertexArray.hpp"
#include <glad/glad.h>

namespace TE {

    OpenGLESVertexArray::OpenGLESVertexArray() {
        glGenVertexArrays(1, &m_RendererID);
        glBindVertexArray(m_RendererID);
    }

    OpenGLESVertexArray::~OpenGLESVertexArray() {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLESVertexArray::Bind() const {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLESVertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLESVertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        // Default layout: location 0 = vec3 position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,                  // layout(location = 0)
            3,                  // vec3
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float),  // stride
            reinterpret_cast<const void*>(0) // offset
        );

        m_VertexBuffer = vertexBuffer;
    }

    void OpenGLESVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
        glBindVertexArray(m_RendererID);
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

} // namespace TE

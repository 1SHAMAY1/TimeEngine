#include "Renderer/OpenGL/OpenGLVertexArray.hpp"
#include <glad/glad.h>

namespace TE {

    OpenGLVertexArray::OpenGLVertexArray() {
        glGenVertexArrays(1, &m_RendererID);
        glBindVertexArray(m_RendererID);

    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLVertexArray::Bind() const {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLVertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer)
    {
        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,                // layout(location = 0)
            3,                // vec3
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float), // stride
            (void*)0           // offset
        );

        m_VertexBuffer = vertexBuffer;
    }



    void OpenGLVertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {
        m_IndexBuffer = indexBuffer;
        glBindVertexArray(m_RendererID);
        indexBuffer->Bind();
    }

}

#include "Layers/TestTriangleLayer.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Log.h"
#include <glm/glm.hpp>
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/TEColor.hpp"

namespace TE {

    TestTriangleLayer::TestTriangleLayer() {
        TE_CORE_INFO("TestTriangleLayer: Constructor called.");
    }

    TestTriangleLayer::~TestTriangleLayer() {
        TE_CORE_INFO("TestTriangleLayer: Destructor called.");
    }

    void TestTriangleLayer::OnAttach() {
        // Initialize RenderCommand system
        RenderCommand::Init();

        // Create Vertex Array
        m_VertexArray.reset(VertexArray::Create());

        // Vertex data: 3 vertices, each with x, y, z
        float vertices[] = {
            0.0f,  0.5f, 0.0f,  // Top
           -0.5f, -0.5f, 0.0f,  // Bottom Left
            0.5f, -0.5f, 0.0f   // Bottom Right
        };

        // Create Vertex Buffer
        m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

        // Bind VAO and set vertex attributes
        m_VertexArray->Bind();
        m_VertexBuffer->Bind();
        m_VertexArray->AddVertexBuffer(m_VertexBuffer.get());

        // Create Index Buffer
        unsigned int indices[] = { 0, 1, 2 };
        m_IndexBuffer.reset(IndexBuffer::Create(indices, 3));

        // Attach IBO to VAO
        m_VertexArray->Bind();
        m_IndexBuffer->Bind();
        m_VertexArray->SetIndexBuffer(m_IndexBuffer.get());

        // Use ShaderLibrary for shader
        m_Shader = ShaderLibrary::CreateColorShader();
    }

    void TestTriangleLayer::OnDetach() {
    }

    void TestTriangleLayer::OnUpdate() {
        m_Shader->Bind();
        // Set color using TEColor
        ShaderLibrary::SetColor(m_Shader.get(), TEColor::Red());
        // Set transform (identity)
        glm::mat4 transform = glm::mat4(1.0f);
        ShaderLibrary::SetTransform(m_Shader.get(), transform);
        // Set view-projection (orthographic for 2D)
        glm::mat4 viewProj = ShaderLibrary::CreateOrthographicMatrix(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        ShaderLibrary::SetViewProjection(m_Shader.get(), viewProj);
        m_VertexArray->Bind();
        RenderCommand::DrawIndexed(m_VertexArray->GetRendererID(), m_IndexBuffer->GetCount());
    }

    void TestTriangleLayer::OnImGuiRender() {
        
    }
}

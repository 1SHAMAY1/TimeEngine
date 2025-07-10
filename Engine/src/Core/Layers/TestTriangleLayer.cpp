#include "Layers/TestTriangleLayer.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Log.h"
#include <glm/glm.hpp>
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/Material.hpp"

namespace TE {

std::shared_ptr<Renderer2D> s_Renderer2D;

    TestTriangleLayer::TestTriangleLayer() {
        TE_CORE_INFO("TestTriangleLayer: Constructor called.");
    }

    TestTriangleLayer::~TestTriangleLayer() {
        TE_CORE_INFO("TestTriangleLayer: Destructor called.");
    }

    void TestTriangleLayer::OnAttach() {
        // Initialize RenderCommand system
        RenderCommand::Init();

        // Create Renderer2D instance
        s_Renderer2D = Renderer2D::Create();

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

        // Create a material for batching
        m_Material = std::make_shared<Material>(m_Shader);
        m_Material->SetColor(TEColor::Red());
    }

    void TestTriangleLayer::OnDetach() {
    }

    void TestTriangleLayer::OnUpdate() {
        s_Renderer2D->BeginFrame();
        // Submit a quad at the triangle's position for demonstration
        s_Renderer2D->SubmitQuad(glm::vec2(-0.5f, -0.5f), glm::vec2(1.0f, 1.0f), m_Material);
        s_Renderer2D->EndFrame();
        s_Renderer2D->Flush();
    }

    void TestTriangleLayer::OnImGuiRender() {
        
    }

    std::shared_ptr<Material> m_Material;
}

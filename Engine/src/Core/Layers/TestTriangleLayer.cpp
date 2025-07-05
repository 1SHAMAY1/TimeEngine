#include "Layers/TestTriangleLayer.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Log.h"

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
        m_VertexArray->Bind();          // Must bind VAO first
        m_IndexBuffer->Bind();          // Then bind IBO while VAO is active
        m_VertexArray->SetIndexBuffer(m_IndexBuffer.get());

        // Create Shader
        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            out vec3 v_Position;
            void main() {
                v_Position = a_Position;
                gl_Position = vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            in vec3 v_Position;
            out vec4 FragColor;
            void main() {
                // Convert from [-0.5, 0.5] to [0, 1]
                vec2 coord = v_Position.xy + vec2(0.5);
                float dist = length(coord - vec2(0.5));
                
                // Low alpha at center (dist = 0), high at edge (dist ≈ 0.7)
                float alpha = smoothstep(0.1, 0.5, dist);
                
                vec3 color = vec3(coord.x, 1.0 - coord.y, 0.6);
                FragColor = vec4(color, alpha);
            }
        )";

        m_Shader.reset(Shader::Create(vertexSrc, fragmentSrc));

    }

    void TestTriangleLayer::OnDetach() {
    }

    void TestTriangleLayer::OnUpdate() {
        m_Shader->Bind();
        m_VertexArray->Bind();
        RenderCommand::DrawIndexed(m_VertexArray->GetRendererID(), m_IndexBuffer->GetCount());
    }

    void TestTriangleLayer::OnImGuiRender() {
        
    }
}

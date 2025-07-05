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

        float vertices[] = {
             0.0f,  0.5f, 0.0f,  // Top
            -0.5f, -0.5f, 0.0f,  // Left
             0.5f, -0.5f, 0.0f   // Right
        };

        uint32_t indices[] = { 0, 1, 2 };

        m_VertexArray = VertexArray::Create();

        auto vb = VertexBuffer::Create(vertices, sizeof(vertices));
        m_VertexArray->AddVertexBuffer(vb);

        auto ib = IndexBuffer::Create(indices, 3);
        m_VertexArray->SetIndexBuffer(ib);

        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            void main() {
                gl_Position = vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            out vec4 FragColor;
            void main() {
                FragColor = vec4(0.9, 0.3, 0.2, 1.0);
            }
        )";

        m_Shader = Shader::Create(vertexSrc, fragmentSrc);

    }

    void TestTriangleLayer::OnDetach() {

        delete m_VertexArray;
        m_VertexArray = nullptr;

        delete m_Shader;
        m_Shader = nullptr;
    }

    void TestTriangleLayer::OnUpdate() {

        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        RenderCommand::Clear();

        m_Shader->Bind();
        m_VertexArray->Bind();
        RenderCommand::DrawIndexed(0, 3);

    }

    void TestTriangleLayer::OnImGuiRender() {
        // Optional: Add GUI logs later
    }
}

#include "Layers/CameraLayer.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Core/Log.h"
#include "Core/KeyCodes.hpp"
#include "Input/Input.hpp"
#include "imgui.h"
#include <chrono>
#include "Core/Events/MouseEvent.h"
#include <cmath> // Required for fabs

namespace TE {

    CameraLayer::CameraLayer() {
        TE_CORE_INFO("CameraLayer: Constructor called.");
    }

    CameraLayer::~CameraLayer() {
        TE_CORE_INFO("CameraLayer: Destructor called.");
    }

    void CameraLayer::OnAttach() {
        TE_CORE_INFO("CameraLayer: OnAttach started.");

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
            
            uniform mat4 u_ViewProjection;
            
            out vec3 v_Position;
            void main() {
                v_Position = a_Position;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

        // Initialize Camera
        m_Camera = std::make_unique<OrthographicCamera>(-1.0f, 1.0f, -1.0f, 1.0f);
        m_CameraController = std::make_unique<CameraController>();

        // Set initial camera position and rotation
        m_Camera->SetPosition(m_CameraPosition);
        m_Camera->SetRotation(m_CameraRotation);
        m_Zoom = 1.0f;
        if (m_Camera)
            m_Camera->SetZoom(m_Zoom);

        TE_CORE_INFO("CameraLayer: OnAttach completed.");
    }

    void CameraLayer::OnDetach() {
        TE_CORE_INFO("CameraLayer: OnDetach called.");
    }

    void CameraLayer::OnUpdate() {
        TE_CORE_INFO("CameraLayer: OnUpdate started.");

        // Calculate delta time
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Handle input with proper delta time
        HandleInput(deltaTime);

        // Update camera
        UpdateCamera();

        // Render
        m_Shader->Bind();
        m_Shader->SetUniformMat4("u_ViewProjection", m_Camera->GetViewProjectionMatrix());
        
        m_VertexArray->Bind();
        RenderCommand::DrawIndexed(m_VertexArray->GetRendererID(), m_IndexBuffer->GetCount());

        TE_CORE_INFO("CameraLayer: OnUpdate completed.");
    }

    void CameraLayer::OnImGuiRender() {
        ImGui::Begin("Camera Controls");

        // Title and instructions
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "🎮 Camera Controls");
        ImGui::Separator();

        // Movement controls
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Movement Controls:");
        ImGui::TextColored(m_WPressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "W - Move Up %s", m_WPressed ? " [ACTIVE]" : "");
        ImGui::TextColored(m_SPressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "S - Move Down %s", m_SPressed ? " [ACTIVE]" : "");
        ImGui::TextColored(m_APressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "A - Move Left %s", m_APressed ? " [ACTIVE]" : "");
        ImGui::TextColored(m_DPressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "D - Move Right %s", m_DPressed ? " [ACTIVE]" : "");
        ImGui::Separator();

        // Rotation controls
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Rotation Controls:");
        ImGui::TextColored(m_QPressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "Q - Rotate Left %s", m_QPressed ? " [ACTIVE]" : "");
        ImGui::TextColored(m_EPressed ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 
            "E - Rotate Right %s", m_EPressed ? " [ACTIVE]" : "");
        ImGui::Separator();

        // Zoom controls
        bool zoomInActive = Input::IsKeyPressed(Key::Z) || Input::IsKeyPressed(Key::KPAdd);
        bool zoomOutActive = Input::IsKeyPressed(Key::X) || Input::IsKeyPressed(Key::KPSubtract);
        auto scrollDelta = Input::GetMouseScrollDelta();
        if (scrollDelta.y < 0) zoomInActive = true;
        if (scrollDelta.y > 0) zoomOutActive = true;

        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Zoom Controls:");
        ImGui::TextColored(zoomInActive ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f,1.0f,1.0f,1.0f), 
            "Z / + / Scroll Up - Zoom In %s", zoomInActive ? "[ACTIVE]" : "");
        ImGui::TextColored(zoomOutActive ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f,1.0f,1.0f,1.0f), 
            "X / - / Scroll Down - Zoom Out %s", zoomOutActive ? "[ACTIVE]" : "");
        ImGui::Text("Current Zoom: %.2f", m_Zoom);
        ImGui::SliderFloat("Zoom Speed", &m_ZoomSpeed, 0.05f, 2.0f, "%.2f");
        ImGui::Separator();

        // Current status
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Current Status:");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
        ImGui::Text("Rotation: %.2f degrees", m_CameraRotation);
        ImGui::Text("Movement Speed: %.2f units/s", m_MovementSpeed);
        ImGui::Text("Rotation Speed: %.2f deg/s", m_RotationSpeed);
        ImGui::Separator();

        // Reset button
        if (ImGui::Button("🔄 Reset Camera", ImVec2(120, 30))) {
            m_CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
            m_CameraRotation = 0.0f;
            m_Zoom = 1.0f;
            if (m_Camera) {
                m_Camera->SetPosition(m_CameraPosition);
                m_Camera->SetRotation(m_CameraRotation);
                m_Camera->SetZoom(m_Zoom);
            }
        }

        // Speed controls
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Speed Settings:");
        ImGui::SliderFloat("Movement Speed", &m_MovementSpeed, 0.5f, 10.0f, "%.2f");
        ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, 10.0f, 200.0f, "%.1f");

        ImGui::Separator();
        ImGui::End();
    }

    void CameraLayer::HandleInput(float deltaTime) {
        // Track key states for visual feedback
        m_WPressed = Input::IsKeyPressed(Key::W);
        m_SPressed = Input::IsKeyPressed(Key::S);
        m_APressed = Input::IsKeyPressed(Key::A);
        m_DPressed = Input::IsKeyPressed(Key::D);
        m_QPressed = Input::IsKeyPressed(Key::Q);
        m_EPressed = Input::IsKeyPressed(Key::E);
        bool zoomInPressed = Input::IsKeyPressed(Key::Z) || Input::IsKeyPressed(Key::KPAdd);
        bool zoomOutPressed = Input::IsKeyPressed(Key::X) || Input::IsKeyPressed(Key::KPSubtract);

        // Handle WASD movement
        if (m_WPressed) {
            m_CameraPosition.y += m_MovementSpeed * deltaTime;
            TE_CORE_INFO("W key pressed - moving up. New position: ({0}, {1}, {2})", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
        }
        
        if (m_SPressed) {
            m_CameraPosition.y -= m_MovementSpeed * deltaTime;
            TE_CORE_INFO("S key pressed - moving down. New position: ({0}, {1}, {2})", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
        }
        
        if (m_APressed) {
            m_CameraPosition.x -= m_MovementSpeed * deltaTime;
            TE_CORE_INFO("A key pressed - moving left. New position: ({0}, {1}, {2})", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
        }
        
        if (m_DPressed) {
            m_CameraPosition.x += m_MovementSpeed * deltaTime;
            TE_CORE_INFO("D key pressed - moving right. New position: ({0}, {1}, {2})", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
        }
        
        // Handle QE rotation
        if (m_QPressed) {
            m_CameraRotation -= m_RotationSpeed * deltaTime;
            TE_CORE_INFO("Q key pressed - rotating left. New rotation: {0}", m_CameraRotation);
        }
        
        if (m_EPressed) {
            m_CameraRotation += m_RotationSpeed * deltaTime;
            TE_CORE_INFO("E key pressed - rotating right. New rotation: {0}", m_CameraRotation);
        }
        // Handle zoom in/out (keyboard)
        float prevZoom = m_Zoom;
        if (zoomInPressed) {
            m_Zoom -= m_ZoomSpeed * deltaTime;
        }
        if (zoomOutPressed) {
            m_Zoom += m_ZoomSpeed * deltaTime;
        }
        // Handle zoom in/out (scroll)
        float scroll = Input::GetMouseScrollDelta().y;
        if (scroll != 0.0f) {
            m_Zoom -= scroll * 0.1f;
        }
        m_Zoom = glm::clamp(m_Zoom, 0.05f, 10.0f);
        if (m_Camera && m_Zoom != prevZoom) {
            m_Camera->SetZoom(m_Zoom);
        }
    }

    void CameraLayer::UpdateCamera() {
        // Update camera position and rotation
        m_Camera->SetPosition(m_CameraPosition);
        m_Camera->SetRotation(m_CameraRotation);
        
        // Update camera controller if needed
        if (m_CameraController) {
            m_CameraController->SetRotation(TERotator(0.0f, m_CameraRotation, 0.0f));
        }
    }

    void CameraLayer::OnEvent(Event& event) {
        // No zoom logic here; handled in HandleInput
    }
} 
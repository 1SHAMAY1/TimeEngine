#pragma once
#include "Layer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Camera/OrthographicCamera.hpp"
#include "Camera/CameraController.hpp"
#include "Input/Input.hpp"

namespace TE {

    class TE_API CameraLayer : public Layer {
    public:
        CameraLayer();
        virtual ~CameraLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;

    private:
        void HandleInput(float deltaTime);
        void UpdateCamera();

    private:
        // Rendering
        std::unique_ptr<VertexBuffer> m_VertexBuffer;
        std::unique_ptr<VertexArray> m_VertexArray;
        std::unique_ptr<IndexBuffer> m_IndexBuffer;
        std::unique_ptr<Shader> m_Shader;

        // Camera
        std::unique_ptr<OrthographicCamera> m_Camera;
        std::unique_ptr<CameraController> m_CameraController;

        // Camera settings
        glm::vec3 m_CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        float m_CameraRotation = 0.0f;
        float m_MovementSpeed = 2.0f; // units per second
        float m_RotationSpeed = 90.0f; // degrees per second
        float m_LastFrameTime = 0.0f;
    };
} 
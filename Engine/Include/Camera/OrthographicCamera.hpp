#pragma once

#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace TE {

    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);

        void SetProjection(float left, float right, float bottom, float top);

        void SetPosition(const glm::vec3& position);
        void SetRotation(float rotation);

        // --- Zoom ---
        void SetZoom(float zoom);
        float GetZoom() const;
        void Zoom(float delta); // Additive zoom

        const glm::vec3& GetPosition() const { return m_Position; }
        float GetRotation() const { return m_Rotation; }

        virtual void RecalculateViewMatrix() override;

    private:
        void UpdateProjection();
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f;
        float m_Zoom = 1.0f;
        float m_Left, m_Right, m_Bottom, m_Top;
    };

}

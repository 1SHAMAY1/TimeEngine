#pragma once

#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace TE {

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip);

        void SetProjection(float fovDegrees, float aspectRatio, float nearClip, float farClip);

        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& eulerDegrees);

        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }

        virtual void RecalculateViewMatrix() override;

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f }; // Euler angles (pitch, yaw, roll)
    };

}

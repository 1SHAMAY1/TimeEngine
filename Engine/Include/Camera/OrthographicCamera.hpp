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

        const glm::vec3& GetPosition() const { return m_Position; }
        float GetRotation() const { return m_Rotation; }

        virtual void RecalculateViewMatrix() override;

    private:
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
        float m_Rotation = 0.0f;
    };

}

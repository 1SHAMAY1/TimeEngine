#include "Camera/PerspectiveCamera.hpp"

namespace TE {

    PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float nearClip, float farClip)
    {
        SetProjection(fov, aspect, nearClip, farClip);
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetProjection(float fov, float aspect, float nearClip, float farClip)
    {
        m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::SetPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetRotation(const glm::vec3& eulerDegrees)
    {
        m_Rotation = eulerDegrees;
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::RecalculateViewMatrix()
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
        transform = glm::rotate(transform, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
        transform = glm::translate(transform, -m_Position);

        m_ViewMatrix = transform;
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

}

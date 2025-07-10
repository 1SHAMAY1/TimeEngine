#include "Camera/OrthographicCamera.hpp"

namespace TE {

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
    {
        m_Left = left;
        m_Right = right;
        m_Bottom = bottom;
        m_Top = top;
        m_Zoom = 1.0f;
        UpdateProjection();
        RecalculateViewMatrix();
    }

    void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
    {
        m_Left = left;
        m_Right = right;
        m_Bottom = bottom;
        m_Top = top;
        UpdateProjection();
    }

    void OrthographicCamera::SetPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateViewMatrix();
    }

    void OrthographicCamera::SetRotation(float rotation)
    {
        m_Rotation = rotation;
        RecalculateViewMatrix();
    }

    void OrthographicCamera::SetZoom(float zoom)
    {
        m_Zoom = glm::clamp(zoom, 0.05f, 10.0f);
        UpdateProjection();
    }

    float OrthographicCamera::GetZoom() const
    {
        return m_Zoom;
    }

    void OrthographicCamera::Zoom(float delta)
    {
        SetZoom(m_Zoom + delta);
    }

    void OrthographicCamera::UpdateProjection()
    {
        float l = m_Left * m_Zoom;
        float r = m_Right * m_Zoom;
        float b = m_Bottom * m_Zoom;
        float t = m_Top * m_Zoom;
        m_ProjectionMatrix = glm::ortho(l, r, b, t, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::RecalculateViewMatrix()
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

}

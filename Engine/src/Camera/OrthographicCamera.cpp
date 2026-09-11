#include "Core/PreRequisites.h"
#include "Camera/OrthographicCamera.hpp"
#include "Utils/Math/MathEngine.hpp"

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

void OrthographicCamera::SetPosition(const TEVector &position)
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
    m_Zoom = (zoom < 0.05f) ? 0.05f : ((zoom > 10.0f) ? 10.0f : zoom);
    UpdateProjection();
}

float OrthographicCamera::GetZoom() const { return m_Zoom; }

void OrthographicCamera::Zoom(float delta) { SetZoom(m_Zoom + delta); }

void OrthographicCamera::UpdateProjection()
{
    float l = m_Left * m_Zoom;
    float r = m_Right * m_Zoom;
    float b = m_Bottom * m_Zoom;
    float t = m_Top * m_Zoom;
    m_ProjectionMatrix = MathEngine::Get().GetActiveAPI()->Ortho(l, r, b, t, -1.0f, 1.0f);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void OrthographicCamera::RecalculateViewMatrix()
{
    float rad = m_Rotation * (3.14159265358979323846f / 180.0f);
    TEMatrix4 translation = MathEngine::Get().GetActiveAPI()->Translate(TEMatrix4(1.0f), m_Position);
    TEMatrix4 rotation = MathEngine::Get().GetActiveAPI()->Rotate(TEMatrix4(1.0f), rad, TEVector(0, 0, 1));
    TEMatrix4 transform = translation * rotation;

    // View matrix is the inverse of camera transform
    TEMatrix4 invTrans = MathEngine::Get().GetActiveAPI()->Translate(
        TEMatrix4(1.0f), TEVector(-m_Position.x, -m_Position.y, -m_Position.z));
    TEMatrix4 invRot = MathEngine::Get().GetActiveAPI()->Rotate(TEMatrix4(1.0f), -rad, TEVector(0, 0, 1));
    m_ViewMatrix = invRot * invTrans;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

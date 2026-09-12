#include "Core/PreRequisites.h"
#include "Camera/PerspectiveCamera.hpp"
#include "Utils/Math/MathEngine.hpp"

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float nearClip, float farClip)
{
    SetProjection(fov, aspect, nearClip, farClip);
    RecalculateViewMatrix();
}

void PerspectiveCamera::SetProjection(float fov, float aspect, float nearClip, float farClip)
{
    float fovRad = fov * (3.14159265358979323846f / 180.0f);
    m_ProjectionMatrix = MathEngine::Get().GetActiveAPI()->Perspective(fovRad, aspect, nearClip, farClip);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void PerspectiveCamera::SetPosition(const TEVector &position)
{
    m_Position = position;
    RecalculateViewMatrix();
}

void PerspectiveCamera::SetRotation(const TEVector &eulerDegrees)
{
    m_Rotation = eulerDegrees;
    RecalculateViewMatrix();
}

void PerspectiveCamera::RecalculateViewMatrix()
{
    float degToRad = 3.14159265358979323846f / 180.0f;
    TEMatrix4 transform = TEMatrix4(1.0f);
    transform = MathEngine::Get().GetActiveAPI()->Rotate(transform, m_Rotation.x * degToRad, TEVector(1, 0, 0));
    transform = MathEngine::Get().GetActiveAPI()->Rotate(transform, m_Rotation.y * degToRad, TEVector(0, 1, 0));
    transform = MathEngine::Get().GetActiveAPI()->Rotate(transform, m_Rotation.z * degToRad, TEVector(0, 0, 1));
    transform =
        MathEngine::Get().GetActiveAPI()->Translate(transform, TEVector(-m_Position.x, -m_Position.y, -m_Position.z));

    m_ViewMatrix = transform;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

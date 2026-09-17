#pragma once

#include "Camera.hpp"
#include "MathUtils.hpp"

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip);

    void SetProjection(float fovDegrees, float aspectRatio, float nearClip, float farClip);

    void SetPosition(const TEVector &position);
    void SetRotation(const TEVector &eulerDegrees);

    const TEVector &GetPosition() const { return m_Position; }
    const TEVector &GetRotation() const { return m_Rotation; }

    virtual void RecalculateViewMatrix() override;

private:
    TEVector m_Position = {0.0f, 0.0f, 0.0f};
    TEVector m_Rotation = {0.0f, 0.0f, 0.0f}; // Euler angles (pitch, yaw, roll)
};

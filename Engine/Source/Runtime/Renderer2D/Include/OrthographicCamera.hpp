#pragma once

#include "Camera.hpp"
#include "MathUtils.hpp"

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera(float left, float right, float bottom, float top);

    void SetProjection(float left, float right, float bottom, float top);

    void SetPosition(const TEVector &position);
    void SetRotation(float rotation);

    // --- Zoom ---
    void SetZoom(float zoom);
    float GetZoom() const;
    void Zoom(float delta); // Additive zoom

    const TEVector &GetPosition() const { return m_Position; }
    float GetRotation() const { return m_Rotation; }

    virtual void RecalculateViewMatrix() override;

private:
    void UpdateProjection();
    TEVector m_Position = {0.0f, 0.0f, 0.0f};
    float m_Rotation = 0.0f;
    float m_Zoom = 1.0f;
    float m_Left, m_Right, m_Bottom, m_Top;
};

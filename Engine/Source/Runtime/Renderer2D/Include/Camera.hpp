#pragma once

#include "PreRequisites.h"
#include "MathUtils.hpp"

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void RecalculateViewMatrix() = 0;

    const TEMatrix4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const TEMatrix4 &GetViewMatrix() const { return m_ViewMatrix; }
    const TEMatrix4 &GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

protected:
    TEMatrix4 m_ProjectionMatrix = TEMatrix4(1.0f);
    TEMatrix4 m_ViewMatrix = TEMatrix4(1.0f);
    TEMatrix4 m_ViewProjectionMatrix = TEMatrix4(1.0f);
};

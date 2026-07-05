#pragma once

#include "Utils/MathUtils.hpp"

namespace TE
{

class CameraController
{
public:
    CameraController();
    ~CameraController() = default;

    // === FOV ===
    void SetFOV(float fov);
    float GetFOV() const;

    // === Rotation Input ===
    void AddPitchInput(float deltaPitch);
    void AddYawInput(float deltaYaw);
    void AddRollInput(float deltaRoll);

    // === Set/Get Rotation ===
    void SetRotation(const TERotator &rotation);
    const TERotator &GetRotation() const;

    // === View ===
    TEMatrix4 GetViewMatrix() const;

private:
    float m_FOV;
    TEVector m_Position;
    TERotator m_Rotation;
};

} // namespace TE

#include "Camera/CameraController.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace TE {

    CameraController::CameraController()
        : m_FOV(45.0f), m_Position(0.0f, 0.0f, 5.0f), m_Rotation(0.0f, 0.0f, 0.0f)
    {
    }

    void CameraController::SetFOV(float fov) {
        m_FOV = glm::clamp(fov, 1.0f, 179.0f);
    }

    float CameraController::GetFOV() const {
        return m_FOV;
    }

    void CameraController::AddPitchInput(float deltaPitch) {
        m_Rotation.AddPitch(deltaPitch);
    }

    void CameraController::AddYawInput(float deltaYaw) {
        m_Rotation.AddYaw(deltaYaw);
    }

    void CameraController::AddRollInput(float deltaRoll) {
        m_Rotation.AddRoll(deltaRoll);
    }

    void CameraController::SetRotation(const TERotator& rotation) {
        m_Rotation = rotation;
    }

    const TERotator& CameraController::GetRotation() const {
        return m_Rotation;
    }

    glm::mat4 CameraController::GetViewMatrix() const {
        glm::mat4 rotation = m_Rotation.ToQuat().ToMatrix();
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_Position);
        return rotation * translation;
    }

}

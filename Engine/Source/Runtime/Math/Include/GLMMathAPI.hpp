#pragma once
#include "PreRequisites.h"
#include "MathAPI.hpp"

/// GLM concrete implementation of MathAPI.
/// The only location where <glm/...> headers are included for MathAPI operations.
class GLMMathAPI final : public MathAPI
{
public:
    GLMMathAPI() = default;
    ~GLMMathAPI() override = default;

    MathBackendType GetType() const override { return MathBackendType::GLM; }

    TEMatrix4 MultiplyMat4(const TEMatrix4 &a, const TEMatrix4 &b) const override;
    TEVector4 MultiplyMat4Vec4(const TEMatrix4 &m, const TEVector4 &v) const override;
    TEMatrix4 Translate(const TEMatrix4 &mat, const TEVector &translation) const override;
    TEMatrix4 Scale(const TEMatrix4 &mat, const TEVector &scale) const override;
    TEMatrix4 Rotate(const TEMatrix4 &mat, float angleRadians, const TEVector &axis) const override;

    TEMatrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar) const override;
    TEMatrix4 Perspective(float fovRadians, float aspect, float zNear, float zFar) const override;
    TEMatrix4 LookAt(const TEVector &eye, const TEVector &center, const TEVector &up) const override;

    TEQuat RotatorToQuat(float pitch, float yaw, float roll) const override;
    TEMatrix4 QuatToMatrix(const TEQuat &q) const override;
    TEQuat QuatFromMatrix(const TEMatrix4 &m) const override;
    TEQuat QuatAngleAxis(float angleRadians, const TEVector &axis) const override;
    TEVector RotateVectorByQuat(const TEQuat &q, const TEVector &v) const override;
    TEMatrix4 TransformToMatrix(const TEVector &pos, const TERotator &rot, const TEVector &scale) const override;
};

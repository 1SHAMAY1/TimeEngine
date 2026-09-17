#pragma once
#include "PreRequisites.h"
#include "MathBackendType.hpp"

// Forward declare value types
struct TEVector2;
struct TEVector;
struct TEVector4;
struct TEMatrix4;
class TERotator;
class TEQuat;
class TETransform;

/// Abstract interface for math and linear algebra operations.
/// Mirrors RendererAPI and UIAPI pattern.
class TE_API MathAPI
{
public:
    virtual ~MathAPI() = default;

    virtual MathBackendType GetType() const = 0;

    // --- Matrix Multiplication & Transform ---
    virtual TEMatrix4 MultiplyMat4(const TEMatrix4 &a, const TEMatrix4 &b) const = 0;
    virtual TEVector4 MultiplyMat4Vec4(const TEMatrix4 &m, const TEVector4 &v) const = 0;
    virtual TEMatrix4 Translate(const TEMatrix4 &mat, const TEVector &translation) const = 0;
    virtual TEMatrix4 Scale(const TEMatrix4 &mat, const TEVector &scale) const = 0;
    virtual TEMatrix4 Rotate(const TEMatrix4 &mat, float angleRadians, const TEVector &axis) const = 0;

    // --- Projections & View Matrices ---
    virtual TEMatrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar) const = 0;
    virtual TEMatrix4 Perspective(float fovRadians, float aspect, float zNear, float zFar) const = 0;
    virtual TEMatrix4 LookAt(const TEVector &eye, const TEVector &center, const TEVector &up) const = 0;

    // --- Quaternions & Rotators ---
    virtual TEQuat RotatorToQuat(float pitch, float yaw, float roll) const = 0;
    virtual TEMatrix4 QuatToMatrix(const TEQuat &q) const = 0;
    virtual TEQuat QuatFromMatrix(const TEMatrix4 &m) const = 0;
    virtual TEQuat QuatAngleAxis(float angleRadians, const TEVector &axis) const = 0;
    virtual TEVector RotateVectorByQuat(const TEQuat &q, const TEVector &v) const = 0;
    virtual TEMatrix4 TransformToMatrix(const TEVector &pos, const TERotator &rot, const TEVector &scale) const = 0;

    // --- Factory ---
    static TEScope<MathAPI> Create(MathBackendType type);
};

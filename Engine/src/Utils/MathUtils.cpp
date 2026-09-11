#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"
#include "Utils/Math/MathEngine.hpp"

// ===== TEMatrix4 Implementation =====
TEMatrix4::TEMatrix4()
{
    // Identity matrix default
    for (int i = 0; i < 4; ++i)
        m[i][i] = 1.0f;
}

TEMatrix4::TEMatrix4(float diagonal)
{
    for (int i = 0; i < 4; ++i)
        m[i][i] = diagonal;
}

TEMatrix4 TEMatrix4::operator*(const TEMatrix4 &other) const
{
    return MathEngine::Get().GetActiveAPI()->MultiplyMat4(*this, other);
}

TEVector4 TEMatrix4::operator*(const TEVector4 &vec) const
{
    return MathEngine::Get().GetActiveAPI()->MultiplyMat4Vec4(*this, vec);
}

// ===== TERotator Implementation =====
TEQuat TERotator::ToQuat() const { return MathEngine::Get().GetActiveAPI()->RotatorToQuat(Pitch, Yaw, Roll); }

// ===== TEQuat Implementation =====
TEMatrix4 TEQuat::ToMatrix() const { return MathEngine::Get().GetActiveAPI()->QuatToMatrix(*this); }

TEQuat TEQuat::FromMatrix(const TEMatrix4 &m) { return MathEngine::Get().GetActiveAPI()->QuatFromMatrix(m); }

TEQuat TEQuat::AngleAxis(float angleRadians, const TEVector &axis)
{
    return MathEngine::Get().GetActiveAPI()->QuatAngleAxis(angleRadians, axis);
}

TEVector TEQuat::operator*(const TEVector &v) const
{
    return MathEngine::Get().GetActiveAPI()->RotateVectorByQuat(*this, v);
}

// ===== TETransform Implementation =====
TEMatrix4 TETransform::GetMatrix() const
{
    return MathEngine::Get().GetActiveAPI()->TransformToMatrix(Position, Rotation, Scale.Scale);
}

TEMatrix4 TEMatrix4::Scale(const TEMatrix4 &mat, const TEVector &scale)
{
    return MathEngine::Get().GetActiveAPI()->Scale(mat, scale);
}

TEMatrix4 TEMatrix4::Translate(const TEMatrix4 &mat, const TEVector &translation)
{
    return MathEngine::Get().GetActiveAPI()->Translate(mat, translation);
}

TEMatrix4 TEMatrix4::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    return MathEngine::Get().GetActiveAPI()->Ortho(left, right, bottom, top, zNear, zFar);
}

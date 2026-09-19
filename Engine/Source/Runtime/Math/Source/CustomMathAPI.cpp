#include "PreRequisites.h"
#include "CustomMathAPI.hpp"
#include "MathUtils.hpp"
#include <cmath>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(_M_IX86)
#include <immintrin.h>
#define TE_SIMD_SSE_SUPPORTED 1
#endif

TEMatrix4 CustomMathAPI::MultiplyMat4(const TEMatrix4 &a, const TEMatrix4 &b) const
{
    // GLM and TimeEngine matrices are column-major (m[col][row])
    TEMatrix4 out(0.0f);

#if defined(TE_SIMD_SSE_SUPPORTED)
    for (int i = 0; i < 4; ++i)
    {
        __m128 colA0 = _mm_loadu_ps(&a.m[0].x);
        __m128 colA1 = _mm_loadu_ps(&a.m[1].x);
        __m128 colA2 = _mm_loadu_ps(&a.m[2].x);
        __m128 colA3 = _mm_loadu_ps(&a.m[3].x);

        __m128 e0 = _mm_set1_ps(b.m[i][0]);
        __m128 e1 = _mm_set1_ps(b.m[i][1]);
        __m128 e2 = _mm_set1_ps(b.m[i][2]);
        __m128 e3 = _mm_set1_ps(b.m[i][3]);

        __m128 res = _mm_add_ps(_mm_add_ps(_mm_mul_ps(colA0, e0), _mm_mul_ps(colA1, e1)),
                                _mm_add_ps(_mm_mul_ps(colA2, e2), _mm_mul_ps(colA3, e3)));

        _mm_storeu_ps(&out.m[i].x, res);
    }
#else
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            out.m[i][j] = a.m[0][j] * b.m[i][0] + a.m[1][j] * b.m[i][1] + a.m[2][j] * b.m[i][2] + a.m[3][j] * b.m[i][3];
        }
    }
#endif
    return out;
}

TEVector4 CustomMathAPI::MultiplyMat4Vec4(const TEMatrix4 &m, const TEVector4 &v) const
{
#if defined(TE_SIMD_SSE_SUPPORTED)
    __m128 col0 = _mm_loadu_ps(&m.m[0].x);
    __m128 col1 = _mm_loadu_ps(&m.m[1].x);
    __m128 col2 = _mm_loadu_ps(&m.m[2].x);
    __m128 col3 = _mm_loadu_ps(&m.m[3].x);

    __m128 vx = _mm_set1_ps(v.x);
    __m128 vy = _mm_set1_ps(v.y);
    __m128 vz = _mm_set1_ps(v.z);
    __m128 vw = _mm_set1_ps(v.w);

    __m128 res = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col0, vx), _mm_mul_ps(col1, vy)),
                            _mm_add_ps(_mm_mul_ps(col2, vz), _mm_mul_ps(col3, vw)));

    alignas(16) float r[4];
    _mm_store_ps(r, res);
    return {r[0], r[1], r[2], r[3]};
#else
    return {m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
            m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
            m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
            m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w};
#endif
}

TEMatrix4 CustomMathAPI::Translate(const TEMatrix4 &mat, const TEVector &t) const
{
    TEMatrix4 res = mat;
    res.m[3] = mat.m[0] * t.x + mat.m[1] * t.y + mat.m[2] * t.z + mat.m[3];
    return res;
}

TEMatrix4 CustomMathAPI::Scale(const TEMatrix4 &mat, const TEVector &s) const
{
    TEMatrix4 res = mat;
    res.m[0] = mat.m[0] * s.x;
    res.m[1] = mat.m[1] * s.y;
    res.m[2] = mat.m[2] * s.z;
    res.m[3] = mat.m[3];
    return res;
}

TEMatrix4 CustomMathAPI::Rotate(const TEMatrix4 &mat, float angleRadians, const TEVector &axis) const
{
    const float c = std::cos(angleRadians);
    const float s = std::sin(angleRadians);
    const TEVector normAxis = axis.Normalized();
    const float x = normAxis.x, y = normAxis.y, z = normAxis.z;
    const float oneMinusC = 1.0f - c;

    TEMatrix4 rot(1.0f);
    rot.m[0][0] = c + x * x * oneMinusC;
    rot.m[0][1] = x * y * oneMinusC + z * s;
    rot.m[0][2] = x * z * oneMinusC - y * s;

    rot.m[1][0] = x * y * oneMinusC - z * s;
    rot.m[1][1] = c + y * y * oneMinusC;
    rot.m[1][2] = y * z * oneMinusC + x * s;

    rot.m[2][0] = x * z * oneMinusC + y * s;
    rot.m[2][1] = y * z * oneMinusC - x * s;
    rot.m[2][2] = c + z * z * oneMinusC;

    return MultiplyMat4(mat, rot);
}

TEMatrix4 CustomMathAPI::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) const
{
    TEMatrix4 res(1.0f);
    res.m[0][0] = 2.0f / (right - left);
    res.m[1][1] = 2.0f / (top - bottom);
    res.m[2][2] = -2.0f / (zFar - zNear);
    res.m[3][0] = -(right + left) / (right - left);
    res.m[3][1] = -(top + bottom) / (top - bottom);
    res.m[3][2] = -(zFar + zNear) / (zFar - zNear);
    return res;
}

TEMatrix4 CustomMathAPI::Perspective(float fovRadians, float aspect, float zNear, float zFar) const
{
    const float tanHalfFov = std::tan(fovRadians / 2.0f);
    TEMatrix4 res(0.0f);
    res.m[0][0] = 1.0f / (aspect * tanHalfFov);
    res.m[1][1] = 1.0f / tanHalfFov;
    res.m[2][2] = -(zFar + zNear) / (zFar - zNear);
    res.m[2][3] = -1.0f;
    res.m[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
    return res;
}

TEMatrix4 CustomMathAPI::LookAt(const TEVector &eye, const TEVector &center, const TEVector &up) const
{
    const TEVector f = (center - eye).Normalized();
    const TEVector s = Cross(f, up).Normalized();
    const TEVector u = Cross(s, f);

    TEMatrix4 res(1.0f);
    res.m[0][0] = s.x;
    res.m[1][0] = s.y;
    res.m[2][0] = s.z;
    res.m[0][1] = u.x;
    res.m[1][1] = u.y;
    res.m[2][1] = u.z;
    res.m[0][2] = -f.x;
    res.m[1][2] = -f.y;
    res.m[2][2] = -f.z;
    res.m[3][0] = -Dot(s, eye);
    res.m[3][1] = -Dot(u, eye);
    res.m[3][2] = Dot(f, eye);
    return res;
}

TEQuat CustomMathAPI::RotatorToQuat(float pitch, float yaw, float roll) const
{
    const float p = Radians(pitch) * 0.5f;
    const float y = Radians(yaw) * 0.5f;
    const float r = Radians(roll) * 0.5f;

    const float sinP = std::sin(p), cosP = std::cos(p);
    const float sinY = std::sin(y), cosY = std::cos(y);
    const float sinR = std::sin(r), cosR = std::cos(r);

    return TEQuat(cosY * sinP * cosR + sinY * cosP * sinR, sinY * cosP * cosR - cosY * sinP * sinR,
                  cosY * cosP * sinR - sinY * sinP * cosR, cosY * cosP * cosR + sinY * sinP * sinR);
}

TEMatrix4 CustomMathAPI::QuatToMatrix(const TEQuat &q) const
{
    TEMatrix4 res(1.0f);
    const float qxx = q.x * q.x;
    const float qyy = q.y * q.y;
    const float qzz = q.z * q.z;
    const float qxz = q.x * q.z;
    const float qxy = q.x * q.y;
    const float qyz = q.y * q.z;
    const float qwx = q.w * q.x;
    const float qwy = q.w * q.y;
    const float qwz = q.w * q.z;

    res.m[0][0] = 1.0f - 2.0f * (qyy + qzz);
    res.m[0][1] = 2.0f * (qxy + qwz);
    res.m[0][2] = 2.0f * (qxz - qwy);

    res.m[1][0] = 2.0f * (qxy - qwz);
    res.m[1][1] = 1.0f - 2.0f * (qxx + qzz);
    res.m[1][2] = 2.0f * (qyz + qwx);

    res.m[2][0] = 2.0f * (qxz + qwy);
    res.m[2][1] = 2.0f * (qyz - qwx);
    res.m[2][2] = 1.0f - 2.0f * (qxx + qyy);

    return res;
}

TEQuat CustomMathAPI::QuatFromMatrix(const TEMatrix4 &m) const
{
    const float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
    if (trace > 0.0f)
    {
        const float s = 0.5f / std::sqrt(trace + 1.0f);
        return TEQuat((m.m[1][2] - m.m[2][1]) * s, (m.m[2][0] - m.m[0][2]) * s, (m.m[0][1] - m.m[1][0]) * s, 0.25f / s);
    }
    else
    {
        if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2])
        {
            const float s = 2.0f * std::sqrt(1.0f + m.m[0][0] - m.m[1][1] - m.m[2][2]);
            return TEQuat(0.25f * s, (m.m[0][1] + m.m[1][0]) / s, (m.m[0][2] + m.m[2][0]) / s,
                          (m.m[1][2] - m.m[2][1]) / s);
        }
        else if (m.m[1][1] > m.m[2][2])
        {
            const float s = 2.0f * std::sqrt(1.0f + m.m[1][1] - m.m[0][0] - m.m[2][2]);
            return TEQuat((m.m[0][1] + m.m[1][0]) / s, 0.25f * s, (m.m[1][2] + m.m[2][1]) / s,
                          (m.m[2][0] - m.m[0][2]) / s);
        }
        else
        {
            const float s = 2.0f * std::sqrt(1.0f + m.m[2][2] - m.m[0][0] - m.m[1][1]);
            return TEQuat((m.m[0][2] + m.m[2][0]) / s, (m.m[1][2] + m.m[2][1]) / s, 0.25f * s,
                          (m.m[0][1] - m.m[1][0]) / s);
        }
    }
}

TEQuat CustomMathAPI::QuatAngleAxis(float angleRadians, const TEVector &axis) const
{
    const float halfAngle = angleRadians * 0.5f;
    const float s = std::sin(halfAngle);
    const TEVector normAxis = axis.Normalized();
    return TEQuat(normAxis.x * s, normAxis.y * s, normAxis.z * s, std::cos(halfAngle));
}

TEVector CustomMathAPI::RotateVectorByQuat(const TEQuat &q, const TEVector &v) const
{
    const TEVector u(q.x, q.y, q.z);
    const float s = q.w;
    return 2.0f * Dot(u, v) * u + (s * s - Dot(u, u)) * v + 2.0f * s * Cross(u, v);
}

TEMatrix4 CustomMathAPI::TransformToMatrix(const TEVector &pos, const TERotator &rot, const TEVector &scale) const
{
    TEMatrix4 rotMat = QuatToMatrix(RotatorToQuat(rot.Pitch, rot.Yaw, rot.Roll));
    TEMatrix4 res = Scale(rotMat, scale);
    res = Translate(res, pos);
    return res;
}

#pragma once

#include <cmath>
#include <sstream>
#include <string>

#include "Core/PreRequisites.h"

// ImVec2 and ImVec4 are forward declared so we don't have to include imgui.h in MathUtils.hpp
struct ImVec2;
struct ImVec4;

namespace TE
{

// ===== TEVector2 =====
struct TE_API TEVector2
{
    float x = 0.0f, y = 0.0f;

    TEVector2() = default;
    TEVector2(float x, float y) : x(x), y(y) {}
    TEVector2(const ImVec2 &v);
    TEVector2(const struct TEVector4 &v);

    static float Length(const TEVector2 &v) { return v.Length(); }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }

    TEVector2 Normalized() const
    {
        float len = Length();
        return (len > 0.0f) ? TEVector2(x / len, y / len) : TEVector2();
    }

    ImVec2 ToImVec2() const;
    operator ImVec2() const;

    TEVector2 operator+(const TEVector2 &rhs) const { return {x + rhs.x, y + rhs.y}; }
    TEVector2 operator-(const TEVector2 &rhs) const { return {x - rhs.x, y - rhs.y}; }
    TEVector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    TEVector2 operator/(float scalar) const { return {x / scalar, y / scalar}; }
    TEVector2 operator-() const { return {-x, -y}; }

    TEVector2 &operator+=(const TEVector2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    TEVector2 &operator-=(const TEVector2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    TEVector2 &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    TEVector2 &operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    bool operator==(const TEVector2 &rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const TEVector2 &rhs) const { return !(*this == rhs); }
};

inline TEVector2 operator*(float scalar, const TEVector2 &v) { return v * scalar; }

inline float Dot(const TEVector2 &a, const TEVector2 &b) { return a.x * b.x + a.y * b.y; }

inline float Distance(const TEVector2 &a, const TEVector2 &b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

inline float DistanceSquared(const TEVector2 &a, const TEVector2 &b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// ===== TEVector (3D) =====
struct TE_API TEVector
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    TEVector() = default;
    TEVector(float x, float y, float z) : x(x), y(y), z(z) {}
    TEVector(const struct TEVector4 &v);

    static float Length(const TEVector &v) { return v.Length(); }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }

    TEVector Normalized() const
    {
        float len = Length();
        return (len > 0.0f) ? TEVector(x / len, y / len, z / len) : TEVector();
    }

    TEVector operator+(const TEVector &rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    TEVector operator-(const TEVector &rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    TEVector operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    TEVector operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }
    TEVector operator-() const { return {-x, -y, -z}; }

    TEVector &operator+=(const TEVector &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    TEVector &operator-=(const TEVector &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    TEVector &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    TEVector &operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    bool operator==(const TEVector &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const TEVector &rhs) const { return !(*this == rhs); }

    bool operator<(const TEVector &rhs) const { return LengthSquared() < rhs.LengthSquared(); }
    bool operator<=(const TEVector &rhs) const { return LengthSquared() <= rhs.LengthSquared(); }
    bool operator>(const TEVector &rhs) const { return LengthSquared() > rhs.LengthSquared(); }
    bool operator>=(const TEVector &rhs) const { return LengthSquared() >= rhs.LengthSquared(); }
};

inline TEVector operator*(float scalar, const TEVector &v) { return v * scalar; }

inline float Distance(const TEVector &a, const TEVector &b)
{
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

inline float DistanceSquared(const TEVector &a, const TEVector &b)
{
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

// ===== TEVector4 =====
struct TE_API TEVector4
{
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

    TEVector4() = default;
    TEVector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    TEVector4(const ImVec4 &v);

    ImVec4 ToImVec4() const;
    operator ImVec4() const;

    float &operator[](int index)
    {
        if (index == 0)
            return x;
        if (index == 1)
            return y;
        if (index == 2)
            return z;
        return w;
    }
    const float &operator[](int index) const
    {
        if (index == 0)
            return x;
        if (index == 1)
            return y;
        if (index == 2)
            return z;
        return w;
    }

    TEVector4 operator+(const TEVector4 &rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w}; }
    TEVector4 operator-(const TEVector4 &rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
    TEVector4 operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
    TEVector4 operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar, w / scalar}; }
    TEVector4 operator-() const { return {-x, -y, -z, -w}; }

    TEVector4 &operator+=(const TEVector4 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    TEVector4 &operator-=(const TEVector4 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    TEVector4 &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }
    TEVector4 &operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    bool operator==(const TEVector4 &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    bool operator!=(const TEVector4 &rhs) const { return !(*this == rhs); }
};

inline TEVector4 operator*(float scalar, const TEVector4 &v) { return v * scalar; }

inline TEVector2::TEVector2(const TEVector4 &v) : x(v.x), y(v.y) {}
inline TEVector::TEVector(const TEVector4 &v) : x(v.x), y(v.y), z(v.z) {}

// ===== TEMatrix4 =====
struct TE_API TEMatrix4
{
    TEVector4 m[4];

    TEMatrix4();
    TEMatrix4(float diagonal);

    TEMatrix4 operator*(const TEMatrix4 &other) const;
    TEVector4 operator*(const TEVector4 &vec) const;

    TEVector4 &operator[](int index) { return m[index]; }
    const TEVector4 &operator[](int index) const { return m[index]; }

    static TEMatrix4 Scale(const TEMatrix4 &mat, const TEVector &scale);
    static TEMatrix4 Translate(const TEMatrix4 &mat, const TEVector &translation);
    static TEMatrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
};

// ===== Forward Declare =====
class TEQuat;

// ===== TERotator =====
class TE_API TERotator
{
public:
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;

    TERotator() = default;
    TERotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

    void AddYaw(float d) { Yaw += d; }
    void AddPitch(float d) { Pitch += d; }
    void AddRoll(float d) { Roll += d; }

    TEVector ToVec3() const { return {Pitch, Yaw, Roll}; }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "Pitch: " << Pitch << ", Yaw: " << Yaw << ", Roll: " << Roll;
        return ss.str();
    }

    bool operator==(const TERotator &other) const
    {
        return Pitch == other.Pitch && Yaw == other.Yaw && Roll == other.Roll;
    }

    bool operator!=(const TERotator &other) const { return !(*this == other); }

    TEQuat ToQuat() const;
};

// ===== TEQuat =====
class TE_API TEQuat
{
public:
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

    TEQuat() = default;
    TEQuat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    TEMatrix4 ToMatrix() const;

    bool operator==(const TEQuat &other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const TEQuat &other) const { return !(*this == other); }
};

// ===== TEScale =====
class TE_API TEScale
{
public:
    TEVector Scale = {1.0f, 1.0f, 1.0f};

    TEScale() = default;
    TEScale(float uniform) : Scale(uniform, uniform, uniform) {}
    TEScale(float x, float y, float z) : Scale(x, y, z) {}
    TEScale(const TEVector &s) : Scale(s) {}

    TEVector Get() const { return Scale; }

    bool operator==(const TEScale &other) const { return Scale == other.Scale; }
    bool operator!=(const TEScale &other) const { return !(*this == other); }
};

// ===== TETransform =====
class TE_API TETransform
{
public:
    TEVector Position = {0.0f, 0.0f, 0.0f};
    TERotator Rotation;
    TEScale Scale = TEScale(1.0f);

    TETransform() = default;

    TEMatrix4 GetMatrix() const;

    bool operator==(const TETransform &other) const
    {
        return Position == other.Position && Rotation == other.Rotation && Scale == other.Scale;
    }

    bool operator!=(const TETransform &other) const { return !(*this == other); }
};

// ===== General Utility Functions =====
template <typename T> inline T Clamp(T value, T min, T max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

inline TEVector2 Normalize(const TEVector2 &v) { return v.Normalized(); }

inline TEVector Normalize(const TEVector &v) { return v.Normalized(); }

template <typename T> inline T Lerp(T a, T b, float t) { return a + (b - a) * t; }

inline float Radians(float degrees) { return degrees * 0.0174532925f; }

inline float Degrees(float radians) { return radians * 57.2957795f; }

template <typename T> inline T Min(T a, T b) { return (a < b) ? a : b; }

template <typename T> inline T Max(T a, T b) { return (a > b) ? a : b; }

template <typename T> inline T Abs(T val) { return (val < 0) ? -val : val; }

inline float Mod(float x, float y) { return std::fmod(x, y); }

inline double Mod(double x, double y) { return std::fmod(x, y); }

inline int Mod(int x, int y) { return x % y; }

// ===== Vector Math Overloads =====

inline float Dot(const TEVector &a, const TEVector &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline TEVector Cross(const TEVector &a, const TEVector &b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline TEVector2 Min(const TEVector2 &a, const TEVector2 &b) { return {Min(a.x, b.x), Min(a.y, b.y)}; }
inline TEVector2 Max(const TEVector2 &a, const TEVector2 &b) { return {Max(a.x, b.x), Max(a.y, b.y)}; }
inline TEVector Min(const TEVector &a, const TEVector &b) { return {Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z)}; }
inline TEVector Max(const TEVector &a, const TEVector &b) { return {Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z)}; }

inline TEVector2 Clamp(const TEVector2 &v, const TEVector2 &min, const TEVector2 &max)
{
    return {Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y)};
}
inline TEVector Clamp(const TEVector &v, const TEVector &min, const TEVector &max)
{
    return {Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y), Clamp(v.z, min.z, max.z)};
}

inline TEVector2 Lerp(const TEVector2 &a, const TEVector2 &b, float t) { return a + (b - a) * t; }
inline TEVector Lerp(const TEVector &a, const TEVector &b, float t) { return a + (b - a) * t; }

inline TEVector2 Mod(const TEVector2 &x, const TEVector2 &y) { return {Mod(x.x, y.x), Mod(x.y, y.y)}; }
inline TEVector Mod(const TEVector &x, const TEVector &y) { return {Mod(x.x, y.x), Mod(x.y, y.y), Mod(x.z, y.z)}; }

} // namespace TE

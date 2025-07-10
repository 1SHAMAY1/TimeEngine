#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "imgui.h"
#include <cmath>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace TE {

    // ===== ImVec2 Operators =====
    inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return { a.x + b.x, a.y + b.y }; }
    inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return { a.x - b.x, a.y - b.y }; }
    inline ImVec2 operator*(const ImVec2& v, float scalar)    { return { v.x * scalar, v.y * scalar }; }
    inline ImVec2 operator*(float scalar, const ImVec2& v)    { return v * scalar; }
    inline ImVec2 operator/(const ImVec2& v, float scalar)    { return { v.x / scalar, v.y / scalar }; }

    inline float Length(const ImVec2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline ImVec2 Normalize(const ImVec2& v) {
        float len = Length(v);
        return (len > 0.0f) ? v / len : ImVec2(0.0f, 0.0f);
    }

    // ===== ImVec4 Operators =====
    inline ImVec4 operator+(const ImVec4& a, const ImVec4& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
    inline ImVec4 operator-(const ImVec4& a, const ImVec4& b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
    inline ImVec4 operator*(const ImVec4& v, float scalar)    { return { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar }; }
    inline ImVec4 operator*(float scalar, const ImVec4& v)    { return v * scalar; }
    inline ImVec4 operator/(const ImVec4& v, float scalar)    { return { v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar }; }

    // ===== TEVector2 =====
    struct TEVector2 {
        float x = 0.0f, y = 0.0f;

        TEVector2() = default;
        TEVector2(float x, float y) : x(x), y(y) {}
        TEVector2(const ImVec2& v) : x(v.x), y(v.y) {}

        float Length() const { return std::sqrt(x * x + y * y); }
        float LengthSquared() const { return x * x + y * y; }

        TEVector2 Normalized() const {
            float len = Length();
            return (len > 0.0f) ? TEVector2(x / len, y / len) : TEVector2();
        }

        ImVec2 ToImVec2() const { return { x, y }; }

        TEVector2 operator+(const TEVector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
        TEVector2 operator-(const TEVector2& rhs) const { return { x - rhs.x, y - rhs.y }; }
        TEVector2 operator*(float scalar) const         { return { x * scalar, y * scalar }; }
        TEVector2 operator/(float scalar) const         { return { x / scalar, y / scalar }; }
        TEVector2 operator-() const                     { return { -x, -y }; }

        TEVector2& operator+=(const TEVector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
        TEVector2& operator-=(const TEVector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
        TEVector2& operator*=(float scalar)         { x *= scalar; y *= scalar; return *this; }
        TEVector2& operator/=(float scalar)         { x /= scalar; y /= scalar; return *this; }

        bool operator==(const TEVector2& rhs) const { return x == rhs.x && y == rhs.y; }
        bool operator!=(const TEVector2& rhs) const { return !(*this == rhs); }
    };

    inline TEVector2 operator*(float scalar, const TEVector2& v) { return v * scalar; }

    inline float Distance(const TEVector2& a, const TEVector2& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    inline float DistanceSquared(const TEVector2& a, const TEVector2& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    // ===== TEVector (3D) =====
    struct TEVector {
        float x = 0.0f, y = 0.0f, z = 0.0f;

        TEVector() = default;
        TEVector(float x, float y, float z) : x(x), y(y), z(z) {}
        TEVector(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}

        float Length() const { return std::sqrt(x * x + y * y + z * z); }
        float LengthSquared() const { return x * x + y * y + z * z; }

        TEVector Normalized() const {
            float len = Length();
            return (len > 0.0f) ? TEVector(x / len, y / len, z / len) : TEVector();
        }

        glm::vec3 ToGLM() const { return { x, y, z }; }

        TEVector operator+(const TEVector& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
        TEVector operator-(const TEVector& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
        TEVector operator*(float scalar) const        { return { x * scalar, y * scalar, z * scalar }; }
        TEVector operator/(float scalar) const        { return { x / scalar, y / scalar, z / scalar }; }
        TEVector operator-() const                    { return { -x, -y, -z }; }

        TEVector& operator+=(const TEVector& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        TEVector& operator-=(const TEVector& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        TEVector& operator*=(float scalar)        { x *= scalar; y *= scalar; z *= scalar; return *this; }
        TEVector& operator/=(float scalar)        { x /= scalar; y /= scalar; z /= scalar; return *this; }

        bool operator==(const TEVector& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
        bool operator!=(const TEVector& rhs) const { return !(*this == rhs); }

        bool operator<(const TEVector& rhs) const  { return LengthSquared() < rhs.LengthSquared(); }
        bool operator<=(const TEVector& rhs) const { return LengthSquared() <= rhs.LengthSquared(); }
        bool operator>(const TEVector& rhs) const  { return LengthSquared() > rhs.LengthSquared(); }
        bool operator>=(const TEVector& rhs) const { return LengthSquared() >= rhs.LengthSquared(); }
    };

    inline TEVector operator*(float scalar, const TEVector& v) { return v * scalar; }

    inline float Distance(const TEVector& a, const TEVector& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline float DistanceSquared(const TEVector& a, const TEVector& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // ===== TEVector4 =====
    struct TEVector4 {
        float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

        TEVector4() = default;
        TEVector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        TEVector4(const ImVec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

        ImVec4 ToImVec4() const { return { x, y, z, w }; }

        TEVector4 operator+(const TEVector4& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
        TEVector4 operator-(const TEVector4& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
        TEVector4 operator*(float scalar) const         { return { x * scalar, y * scalar, z * scalar, w * scalar }; }
        TEVector4 operator/(float scalar) const         { return { x / scalar, y / scalar, z / scalar, w / scalar }; }
        TEVector4 operator-() const                     { return { -x, -y, -z, -w }; }

        TEVector4& operator+=(const TEVector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        TEVector4& operator-=(const TEVector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        TEVector4& operator*=(float scalar)         { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        TEVector4& operator/=(float scalar)         { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

        bool operator==(const TEVector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
        bool operator!=(const TEVector4& rhs) const { return !(*this == rhs); }
    };

    inline TEVector4 operator*(float scalar, const TEVector4& v) { return v * scalar; }

    // ===== Forward Declare =====
    class TEQuat;

    // ===== TERotator =====
    class TERotator {
    public:
        float Pitch = 0.0f;
        float Yaw   = 0.0f;
        float Roll  = 0.0f;

        TERotator() = default;
        TERotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}

        void AddYaw(float d)   { Yaw += d; }
        void AddPitch(float d) { Pitch += d; }
        void AddRoll(float d)  { Roll += d; }

        glm::vec3 ToVec3() const { return { Pitch, Yaw, Roll }; }

        std::string ToString() const {
            std::stringstream ss;
            ss << "Pitch: " << Pitch << ", Yaw: " << Yaw << ", Roll: " << Roll;
            return ss.str();
        }

        bool operator==(const TERotator& other) const {
            return Pitch == other.Pitch && Yaw == other.Yaw && Roll == other.Roll;
        }

        bool operator!=(const TERotator& other) const {
            return !(*this == other);
        }

        TEQuat ToQuat() const;
    };

    // ===== TEQuat =====
    class TEQuat {
    public:
        glm::quat Quaternion;

        TEQuat() : Quaternion(1, 0, 0, 0) {}
        TEQuat(const glm::quat& q) : Quaternion(q) {}

        glm::mat4 ToMatrix() const { return glm::mat4(Quaternion); }

        bool operator==(const TEQuat& other) const { return Quaternion == other.Quaternion; }
        bool operator!=(const TEQuat& other) const { return !(*this == other); }
    };

    inline TEQuat TERotator::ToQuat() const {
        glm::mat4 rot = glm::eulerAngleYXZ(glm::radians(Yaw), glm::radians(Pitch), glm::radians(Roll));
        glm::quat quat = glm::quat_cast(rot);
        return TEQuat(quat);
    }

    // ===== TEScale =====
    class TEScale {
    public:
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TEScale() = default;
        TEScale(float uniform) : Scale(uniform) {}
        TEScale(float x, float y, float z) : Scale(x, y, z) {}
        TEScale(const glm::vec3& s) : Scale(s) {}

        glm::vec3 Get() const { return Scale; }

        bool operator==(const TEScale& other) const { return Scale == other.Scale; }
        bool operator!=(const TEScale& other) const { return !(*this == other); }
    };

    // ===== TETransform =====
    class TETransform {
    public:
        glm::vec3 Position = glm::vec3(0.0f);
        TERotator Rotation;
        TEScale Scale = TEScale(1.0f);

        TETransform() = default;

        glm::mat4 GetMatrix() const {
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), Position);
            glm::mat4 rotation = glm::eulerAngleYXZ(
                glm::radians(Rotation.Yaw),
                glm::radians(Rotation.Pitch),
                glm::radians(Rotation.Roll)
            );
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale.Get());
            return translation * rotation * scale;
        }

        bool operator==(const TETransform& other) const {
            return Position == other.Position &&
                   Rotation == other.Rotation &&
                   Scale == other.Scale;
        }

        bool operator!=(const TETransform& other) const {
            return !(*this == other);
        }
    };

} 

#define GLM_ENABLE_EXPERIMENTAL
#include "Utils/MathUtils.hpp"
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace TE
{

// ===== TEVector2 ImGui Conversion =====
TEVector2::TEVector2(const ImVec2 &v) : x(v.x), y(v.y) {}
ImVec2 TEVector2::ToImVec2() const { return {x, y}; }
TEVector2::operator ImVec2() const { return {x, y}; }

// ===== TEVector4 ImGui Conversion =====
TEVector4::TEVector4(const ImVec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
ImVec4 TEVector4::ToImVec4() const { return {x, y, z, w}; }
TEVector4::operator ImVec4() const { return {x, y, z, w}; }

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
    glm::mat4 a = glm::make_mat4(&m[0][0]);
    glm::mat4 b = glm::make_mat4(&other.m[0][0]);
    glm::mat4 result = a * b;

    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEVector4 TEMatrix4::operator*(const TEVector4 &vec) const
{
    glm::mat4 a = glm::make_mat4(&m[0][0]);
    glm::vec4 b(vec.x, vec.y, vec.z, vec.w);
    glm::vec4 result = a * b;
    return {result.x, result.y, result.z, result.w};
}

// ===== TERotator Implementation =====
TEQuat TERotator::ToQuat() const
{
    glm::mat4 rot = glm::eulerAngleYXZ(glm::radians(Yaw), glm::radians(Pitch), glm::radians(Roll));
    glm::quat quat = glm::quat_cast(rot);
    return TEQuat(quat.x, quat.y, quat.z, quat.w);
}

// ===== TEQuat Implementation =====
TEMatrix4 TEQuat::ToMatrix() const
{
    glm::quat q(w, x, y, z); // Note: glm::quat constructor takes w, x, y, z
    glm::mat4 result = glm::mat4_cast(q);

    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

// ===== TETransform Implementation =====
TEMatrix4 TETransform::GetMatrix() const
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
    glm::mat4 rotation =
        glm::eulerAngleYXZ(glm::radians(Rotation.Yaw), glm::radians(Rotation.Pitch), glm::radians(Rotation.Roll));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(Scale.Scale.x, Scale.Scale.y, Scale.Scale.z));

    glm::mat4 result = translation * rotation * scale;

    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 TEMatrix4::Scale(const TEMatrix4 &mat, const TEVector &scale)
{
    glm::mat4 a = glm::make_mat4(&mat.m[0][0]);
    glm::mat4 result = glm::scale(a, glm::vec3(scale.x, scale.y, scale.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 TEMatrix4::Translate(const TEMatrix4 &mat, const TEVector &translation)
{
    glm::mat4 a = glm::make_mat4(&mat.m[0][0]);
    glm::mat4 result = glm::translate(a, glm::vec3(translation.x, translation.y, translation.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 TEMatrix4::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    glm::mat4 result = glm::ortho(left, right, bottom, top, zNear, zFar);
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

} // namespace TE

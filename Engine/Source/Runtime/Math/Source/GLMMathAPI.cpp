#include "PreRequisites.h"
#include "GLMMathAPI.hpp"
#include "MathUtils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

TEMatrix4 GLMMathAPI::MultiplyMat4(const TEMatrix4 &a, const TEMatrix4 &b) const
{
    glm::mat4 gm_a = glm::make_mat4(&a.m[0][0]);
    glm::mat4 gm_b = glm::make_mat4(&b.m[0][0]);
    glm::mat4 result = gm_a * gm_b;

    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEVector4 GLMMathAPI::MultiplyMat4Vec4(const TEMatrix4 &m, const TEVector4 &v) const
{
    glm::mat4 gm_m = glm::make_mat4(&m.m[0][0]);
    glm::vec4 gm_v(v.x, v.y, v.z, v.w);
    glm::vec4 result = gm_m * gm_v;
    return {result.x, result.y, result.z, result.w};
}

TEMatrix4 GLMMathAPI::Translate(const TEMatrix4 &mat, const TEVector &translation) const
{
    glm::mat4 a = glm::make_mat4(&mat.m[0][0]);
    glm::mat4 result = glm::translate(a, glm::vec3(translation.x, translation.y, translation.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 GLMMathAPI::Scale(const TEMatrix4 &mat, const TEVector &scale) const
{
    glm::mat4 a = glm::make_mat4(&mat.m[0][0]);
    glm::mat4 result = glm::scale(a, glm::vec3(scale.x, scale.y, scale.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 GLMMathAPI::Rotate(const TEMatrix4 &mat, float angleRadians, const TEVector &axis) const
{
    glm::mat4 a = glm::make_mat4(&mat.m[0][0]);
    glm::mat4 result = glm::rotate(a, angleRadians, glm::vec3(axis.x, axis.y, axis.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 GLMMathAPI::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) const
{
    glm::mat4 result = glm::ortho(left, right, bottom, top, zNear, zFar);
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 GLMMathAPI::Perspective(float fovRadians, float aspect, float zNear, float zFar) const
{
    glm::mat4 result = glm::perspective(fovRadians, aspect, zNear, zFar);
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEMatrix4 GLMMathAPI::LookAt(const TEVector &eye, const TEVector &center, const TEVector &up) const
{
    glm::mat4 result = glm::lookAt(glm::vec3(eye.x, eye.y, eye.z), glm::vec3(center.x, center.y, center.z),
                                   glm::vec3(up.x, up.y, up.z));
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEQuat GLMMathAPI::RotatorToQuat(float pitch, float yaw, float roll) const
{
    glm::mat4 rot = glm::eulerAngleYXZ(glm::radians(yaw), glm::radians(pitch), glm::radians(roll));
    glm::quat quat = glm::quat_cast(rot);
    return TEQuat(quat.x, quat.y, quat.z, quat.w);
}

TEMatrix4 GLMMathAPI::QuatToMatrix(const TEQuat &q) const
{
    glm::quat gq(q.w, q.x, q.y, q.z);
    glm::mat4 result = glm::mat4_cast(gq);
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

TEQuat GLMMathAPI::QuatFromMatrix(const TEMatrix4 &m) const
{
    glm::mat4 gm = glm::make_mat4(&m.m[0][0]);
    glm::quat q = glm::quat_cast(gm);
    return TEQuat(q.x, q.y, q.z, q.w);
}

TEQuat GLMMathAPI::QuatAngleAxis(float angleRadians, const TEVector &axis) const
{
    glm::quat q = glm::angleAxis(angleRadians, glm::vec3(axis.x, axis.y, axis.z));
    return TEQuat(q.x, q.y, q.z, q.w);
}

TEVector GLMMathAPI::RotateVectorByQuat(const TEQuat &q, const TEVector &v) const
{
    glm::quat gq(q.w, q.x, q.y, q.z);
    glm::vec3 gv(v.x, v.y, v.z);
    glm::vec3 result = gq * gv;
    return TEVector(result.x, result.y, result.z);
}

TEMatrix4 GLMMathAPI::TransformToMatrix(const TEVector &pos, const TERotator &rot, const TEVector &scale) const
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
    glm::mat4 rotation = glm::eulerAngleYXZ(glm::radians(rot.Yaw), glm::radians(rot.Pitch), glm::radians(rot.Roll));
    glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

    glm::mat4 result = translation * rotation * scaling;
    TEMatrix4 ret;
    memcpy(&ret.m[0][0], glm::value_ptr(result), 16 * sizeof(float));
    return ret;
}

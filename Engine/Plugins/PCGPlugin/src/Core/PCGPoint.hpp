#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <variant>

enum class PCGAttributeType
{
    Float,
    Int,
    String,
    Vec2,
    Vec3,
    Vec4,
    Bool
};

struct PCGAttributeValue
{
    PCGAttributeType Type = PCGAttributeType::Float;
    std::variant<float, int, TEString, glm::vec2, glm::vec3, glm::vec4, bool> Value;

    PCGAttributeValue() : Type(PCGAttributeType::Float), Value(0.0f) {}
    PCGAttributeValue(float v) : Type(PCGAttributeType::Float), Value(v) {}
    PCGAttributeValue(int v) : Type(PCGAttributeType::Int), Value(v) {}
    PCGAttributeValue(const TEString &v) : Type(PCGAttributeType::String), Value(v) {}
    PCGAttributeValue(const glm::vec2 &v) : Type(PCGAttributeType::Vec2), Value(v) {}
    PCGAttributeValue(const glm::vec3 &v) : Type(PCGAttributeType::Vec3), Value(v) {}
    PCGAttributeValue(const glm::vec4 &v) : Type(PCGAttributeType::Vec4), Value(v) {}
    PCGAttributeValue(bool v) : Type(PCGAttributeType::Bool), Value(v) {}

    float AsFloat() const
    {
        if (std::holds_alternative<float>(Value))
            return std::get<float>(Value);
        if (std::holds_alternative<int>(Value))
            return static_cast<float>(std::get<int>(Value));
        return 0.0f;
    }

    int AsInt() const
    {
        if (std::holds_alternative<int>(Value))
            return std::get<int>(Value);
        if (std::holds_alternative<float>(Value))
            return static_cast<int>(std::get<float>(Value));
        return 0;
    }

    TEString AsString() const
    {
        if (std::holds_alternative<TEString>(Value))
            return std::get<TEString>(Value);
        return "";
    }
};

struct PCGPoint
{
    glm::vec3 Position = {0.0f, 0.0f, 0.0f};
    glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 Scale = {1.0f, 1.0f, 1.0f};
    glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f};

    float Density = 1.0f;
    float BoundsRadius = 0.5f;
    uint32_t Seed = 0;
    uint64_t ID = 0;

    TEMap<TEString, PCGAttributeValue> Attributes;

    PCGPoint() = default;
    PCGPoint(const glm::vec3 &pos, float density = 1.0f, float radius = 0.5f)
        : Position(pos), Density(density), BoundsRadius(radius)
    {
    }

    void SetAttribute(const TEString &key, const PCGAttributeValue &val) { Attributes[key] = val; }

    const PCGAttributeValue *GetAttribute(const TEString &key) const { return Attributes.Find(key); }

    bool HasAttribute(const TEString &key) const { return Attributes.Find(key) != nullptr; }
};

#pragma once
#include "Renderer/TEColor.hpp"
#include "Utility/MathUtils.hpp"
#include "Utility/UIUtils.hpp"
#include "imgui.h"
#include <memory>
#include <string>
#include <vector>

namespace TE
{

template <typename T> struct TEPropertyDrawer
{
    static void Draw(void *addr, const std::string &displayName)
    {
        ImGui::Text("%s: [No Drawer]", displayName.c_str());
    }
    static std::string Serialize(void *addr) { return ""; }
    static void Deserialize(void *addr, const std::string &data) {}
};

template <> struct TEPropertyDrawer<uint64_t>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        ImGui::Text("%s: %llu", displayName.c_str(), *(uint64_t *)addr);
    }
    static std::string Serialize(void *addr) { return std::to_string(*(uint64_t *)addr); }
    static void Deserialize(void *addr, const std::string &data)
    {
        if (!data.empty() && data != "0")
            *(uint64_t *)addr = std::stoull(data);
        else
            *(uint64_t *)addr = 0;
    }
};

template <> struct TEPropertyDrawer<float>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        ImGui::DragFloat(displayName.c_str(), (float *)addr, 0.1f);
    }
    static std::string Serialize(void *addr) { return std::to_string(*(float *)addr); }
    static void Deserialize(void *addr, const std::string &data) { if (!data.empty()) *(float *)addr = std::stof(data); }
};

template <> struct TEPropertyDrawer<int>
{
    static void Draw(void *addr, const std::string &displayName) { ImGui::DragInt(displayName.c_str(), (int *)addr); }
    static std::string Serialize(void *addr) { return std::to_string(*(int *)addr); }
    static void Deserialize(void *addr, const std::string &data) { if (!data.empty()) *(int *)addr = std::stoi(data); }
};

template <> struct TEPropertyDrawer<bool>
{
    static void Draw(void *addr, const std::string &displayName) { ImGui::Checkbox(displayName.c_str(), (bool *)addr); }
    static std::string Serialize(void *addr) { return *(bool *)addr ? "true" : "false"; }
    static void Deserialize(void *addr, const std::string &data) { *(bool *)addr = (data == "true" || data == "1"); }
};

template <> struct TEPropertyDrawer<TEVector2>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec2Control(displayName, *(glm::vec2 *)addr);
    }
    static std::string Serialize(void *addr) { auto v = *(TEVector2 *)addr; return std::to_string(v.x) + " " + std::to_string(v.y); }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        auto *v = (TEVector2 *)addr;
        ss >> v->x >> v->y;
    }
};

template <> struct TEPropertyDrawer<glm::vec2>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec2Control(displayName, *(glm::vec2 *)addr);
    }
    static std::string Serialize(void *addr) { auto v = *(glm::vec2 *)addr; return std::to_string(v.x) + " " + std::to_string(v.y); }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        auto *v = (glm::vec2 *)addr;
        ss >> v->x >> v->y;
    }
};

template <> struct TEPropertyDrawer<glm::vec3>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec3Control(displayName, *(glm::vec3 *)addr);
    }
    static std::string Serialize(void *addr) { auto v = *(glm::vec3 *)addr; return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        auto *v = (glm::vec3 *)addr;
        ss >> v->x >> v->y >> v->z;
    }
};

template <> struct TEPropertyDrawer<TEColor>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawColorControl(displayName, ((TEColor *)addr)->GetValue());
    }
    static std::string Serialize(void *addr) { auto v = ((TEColor *)addr)->GetValue(); return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w); }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        glm::vec4 v;
        ss >> v.x >> v.y >> v.z >> v.w;
        ((TEColor *)addr)->GetValue() = v;
    }
};

template <> struct TEPropertyDrawer<TERotator>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        TERotator *rot = (TERotator *)addr;
        glm::vec3 euler = rot->ToVec3();
        if (ImGui::DragFloat3(displayName.c_str(), &euler.x, 0.1f))
        {
            rot->Pitch = euler.x;
            rot->Yaw = euler.y;
            rot->Roll = euler.z;
        }
    }
    static std::string Serialize(void *addr) { 
        TERotator *rot = (TERotator *)addr;
        return std::to_string(rot->Pitch) + " " + std::to_string(rot->Yaw) + " " + std::to_string(rot->Roll); 
    }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        TERotator *rot = (TERotator *)addr;
        ss >> rot->Pitch >> rot->Yaw >> rot->Roll;
    }
};

template <> struct TEPropertyDrawer<TEScale>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        TEScale *scale = (TEScale *)addr;
        ImGui::DragFloat3(displayName.c_str(), &scale->Scale.x, 0.1f);
    }
    static std::string Serialize(void *addr) { 
        TEScale *scale = (TEScale *)addr;
        return std::to_string(scale->Scale.x) + " " + std::to_string(scale->Scale.y) + " " + std::to_string(scale->Scale.z); 
    }
    static void Deserialize(void *addr, const std::string &data) { 
        std::stringstream ss(data);
        TEScale *scale = (TEScale *)addr;
        ss >> scale->Scale.x >> scale->Scale.y >> scale->Scale.z;
    }
};

template <> struct TEPropertyDrawer<TETransform>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        TETransform *transform = (TETransform *)addr;
        TEPropertyDrawer<glm::vec3>::Draw(&transform->Position, "Position");
        TEPropertyDrawer<TERotator>::Draw(&transform->Rotation, "Rotation");
        TEPropertyDrawer<TEScale>::Draw(&transform->Scale, "Scale");
    }
    static std::string Serialize(void *addr) { 
        return ""; // Not used directly for the structural component layout I implemented
    }
    static void Deserialize(void *addr, const std::string &data) { 
        // Not used directly
    }
};

// Helper for Enums (requires manual specialization or a common base)
// For now, we can treat them as Int if needed, or provide a specific drawer.

} // namespace TE

#pragma once
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <memory>
#include <string>
#include <vector>

namespace TE
{

template <typename T> struct TEPropertyDrawer
{
    static void Draw(void *addr, const std::string &displayName) { TimeGUI::Text(displayName + ": [No Drawer]"); }
    static std::string Serialize(void *addr) { return ""; }
    static void Deserialize(void *addr, const std::string &data) {}
};

template <> struct TEPropertyDrawer<uint64_t>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        TimeGUI::Text(displayName + ": " + std::to_string(*(uint64_t *)addr));
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
        TimeGUI::DragFloat(displayName, (float *)addr, 0.1f);
    }
    static std::string Serialize(void *addr) { return std::to_string(*(float *)addr); }
    static void Deserialize(void *addr, const std::string &data)
    {
        if (!data.empty())
            *(float *)addr = std::stof(data);
    }
};

template <> struct TEPropertyDrawer<int>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        // Forward to standard drag float or standard draw
    }
    static std::string Serialize(void *addr) { return std::to_string(*(int *)addr); }
    static void Deserialize(void *addr, const std::string &data)
    {
        if (!data.empty())
            *(int *)addr = std::stoi(data);
    }
};

template <> struct TEPropertyDrawer<bool>
{
    static void Draw(void *addr, const std::string &displayName) { TimeGUI::Checkbox(displayName, (bool *)addr); }
    static std::string Serialize(void *addr) { return *(bool *)addr ? "true" : "false"; }
    static void Deserialize(void *addr, const std::string &data) { *(bool *)addr = (data == "true" || data == "1"); }
};

template <> struct TEPropertyDrawer<TEVector2>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec2Control(displayName, *(TEVector2 *)addr);
    }
    static std::string Serialize(void *addr)
    {
        auto v = *(TEVector2 *)addr;
        return std::to_string(v.x) + " " + std::to_string(v.y);
    }
    static void Deserialize(void *addr, const std::string &data)
    {
        std::stringstream ss(data);
        auto *v = (TEVector2 *)addr;
        ss >> v->x >> v->y;
    }
};

template <> struct TEPropertyDrawer<TEVector>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec3Control(displayName, *(TEVector *)addr);
    }
    static std::string Serialize(void *addr)
    {
        auto v = *(TEVector *)addr;
        return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z);
    }
    static void Deserialize(void *addr, const std::string &data)
    {
        std::stringstream ss(data);
        auto *v = (TEVector *)addr;
        ss >> v->x >> v->y >> v->z;
    }
};

template <> struct TEPropertyDrawer<TEColor>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawColorControl(displayName, ((TEColor *)addr)->GetValue());
    }
    static std::string Serialize(void *addr)
    {
        auto v = ((TEColor *)addr)->GetValue();
        return std::to_string(v.r) + " " + std::to_string(v.g) + " " + std::to_string(v.b) + " " + std::to_string(v.a);
    }
    static void Deserialize(void *addr, const std::string &data)
    {
        std::stringstream ss(data);
        auto *v = &((TEColor *)addr)->GetValue();
        ss >> v->r >> v->g >> v->b >> v->a;
    }
};

template <> struct TEPropertyDrawer<TERotator>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        TERotator *rot = (TERotator *)addr;
        TEVector euler = rot->ToVec3();
        if (UIUtils::DrawVec3Control(displayName, euler, 0.0f))
        {
            rot->Pitch = euler.x;
            rot->Yaw = euler.y;
            rot->Roll = euler.z;
        }
    }
    static std::string Serialize(void *addr)
    {
        TERotator *rot = (TERotator *)addr;
        return std::to_string(rot->Pitch) + " " + std::to_string(rot->Yaw) + " " + std::to_string(rot->Roll);
    }
    static void Deserialize(void *addr, const std::string &data)
    {
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
        UIUtils::DrawVec3Control(displayName, scale->Scale, 1.0f);
    }
    static std::string Serialize(void *addr)
    {
        TEScale *scale = (TEScale *)addr;
        return std::to_string(scale->Scale.x) + " " + std::to_string(scale->Scale.y) + " " +
               std::to_string(scale->Scale.z);
    }
    static void Deserialize(void *addr, const std::string &data)
    {
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
        TEPropertyDrawer<TEVector>::Draw(&transform->Position, "Position");
        TEPropertyDrawer<TERotator>::Draw(&transform->Rotation, "Rotation");
        TEPropertyDrawer<TEScale>::Draw(&transform->Scale, "Scale");
    }
    static std::string Serialize(void *addr)
    {
        return ""; // Not used directly
    }
    static void Deserialize(void *addr, const std::string &data)
    {
        // Not used directly
    }
};

} // namespace TE

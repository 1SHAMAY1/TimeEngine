#pragma once
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include "Utils/TimeGUI.hpp"

template <typename T> struct TEPropertyDrawer
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TimeGUI::Text(displayName + ": [No Drawer]");
        return false;
    }
    static TEString Serialize(void *addr) { return ""; }
    static void Deserialize(void *addr, const TEString &data) {}
};

template <> struct TEPropertyDrawer<uint64_t>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TEString cleanName = displayName;
        size_t hashPos = cleanName.find("###");
        if (hashPos != TEString::npos)
            cleanName = cleanName.substr(0, hashPos);
        TimeGUI::Text(cleanName + ": " + TEString::FromInt64(static_cast<int64_t>(*(uint64_t *)addr)));
        return false;
    }
    static TEString Serialize(void *addr) { return TEString::FromInt64(static_cast<int64_t>(*(uint64_t *)addr)); }
    static void Deserialize(void *addr, const TEString &data)
    {
        if (!data.empty() && data != "0")
            *(uint64_t *)addr = static_cast<uint64_t>(data.ToInt64());
        else
            *(uint64_t *)addr = 0;
    }
};

template <> struct TEPropertyDrawer<float>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        return TimeGUI::DragFloat(displayName, (float *)addr, 0.1f);
    }
    static TEString Serialize(void *addr) { return TEString::FromFloat(*(float *)addr); }
    static void Deserialize(void *addr, const TEString &data)
    {
        if (!data.empty())
            *(float *)addr = data.ToFloat();
    }
};

template <> struct TEPropertyDrawer<int>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        return TimeGUI::DragInt(displayName, (int *)addr, 1.0f);
    }
    static TEString Serialize(void *addr) { return TEString::FromInt(*(int *)addr); }
    static void Deserialize(void *addr, const TEString &data)
    {
        if (!data.empty())
            *(int *)addr = data.ToInt();
    }
};

template <> struct TEPropertyDrawer<TEString>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TEString *str = (TEString *)addr;
        return TimeGUI::InputText(displayName, *str);
    }
    static TEString Serialize(void *addr) { return *(TEString *)addr; }
    static void Deserialize(void *addr, const TEString &data) { *(TEString *)addr = data; }
};

template <> struct TEPropertyDrawer<bool>
{
    static bool Draw(void *addr, const TEString &displayName) { return TimeGUI::Checkbox(displayName, (bool *)addr); }
    static TEString Serialize(void *addr) { return *(bool *)addr ? "true" : "false"; }
    static void Deserialize(void *addr, const TEString &data) { *(bool *)addr = (data == "true" || data == "1"); }
};

template <> struct TEPropertyDrawer<TEVector2>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        return UIUtils::DrawVec2Control(displayName, *(TEVector2 *)addr);
    }
    static TEString Serialize(void *addr)
    {
        auto v = *(TEVector2 *)addr;
        return TEString::FromFloat(v.x) + " " + TEString::FromFloat(v.y);
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        auto parts = data.Split(" ");
        if (parts.size() >= 2)
        {
            auto *v = (TEVector2 *)addr;
            v->x = parts[0].ToFloat();
            v->y = parts[1].ToFloat();
        }
    }
};

template <> struct TEPropertyDrawer<TEVector>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        return UIUtils::DrawVec3Control(displayName, *(TEVector *)addr);
    }
    static TEString Serialize(void *addr)
    {
        auto v = *(TEVector *)addr;
        return TEString::FromFloat(v.x) + " " + TEString::FromFloat(v.y) + " " + TEString::FromFloat(v.z);
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        auto parts = data.Split(" ");
        if (parts.size() >= 3)
        {
            auto *v = (TEVector *)addr;
            v->x = parts[0].ToFloat();
            v->y = parts[1].ToFloat();
            v->z = parts[2].ToFloat();
        }
    }
};

template <> struct TEPropertyDrawer<TEColor>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        return UIUtils::DrawColorControl(displayName, *(TEColor *)addr);
    }
    static TEString Serialize(void *addr)
    {
        auto v = ((TEColor *)addr)->GetValue();
        return TEString::FromFloat(v.r) + " " + TEString::FromFloat(v.g) + " " + TEString::FromFloat(v.b) + " " +
               TEString::FromFloat(v.a);
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        auto parts = data.Split(" ");
        if (parts.size() >= 4)
        {
            auto *v = &((TEColor *)addr)->GetValue();
            v->r = parts[0].ToFloat();
            v->g = parts[1].ToFloat();
            v->b = parts[2].ToFloat();
            v->a = parts[3].ToFloat();
        }
    }
};

template <> struct TEPropertyDrawer<TERotator>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TERotator *rot = (TERotator *)addr;
        TEVector euler = rot->ToVec3();
        if (UIUtils::DrawVec3Control(displayName, euler, 0.0f))
        {
            rot->Pitch = euler.x;
            rot->Yaw = euler.y;
            rot->Roll = euler.z;
            return true;
        }
        return false;
    }
    static TEString Serialize(void *addr)
    {
        TERotator *rot = (TERotator *)addr;
        return TEString::FromFloat(rot->Pitch) + " " + TEString::FromFloat(rot->Yaw) + " " +
               TEString::FromFloat(rot->Roll);
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        auto parts = data.Split(" ");
        if (parts.size() >= 3)
        {
            TERotator *rot = (TERotator *)addr;
            rot->Pitch = parts[0].ToFloat();
            rot->Yaw = parts[1].ToFloat();
            rot->Roll = parts[2].ToFloat();
        }
    }
};

template <> struct TEPropertyDrawer<TEScale>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TEScale *scale = (TEScale *)addr;
        return UIUtils::DrawVec3Control(displayName, scale->Scale, 1.0f);
    }
    static TEString Serialize(void *addr)
    {
        TEScale *scale = (TEScale *)addr;
        return TEString::FromFloat(scale->Scale.x) + " " + TEString::FromFloat(scale->Scale.y) + " " +
               TEString::FromFloat(scale->Scale.z);
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        auto parts = data.Split(" ");
        if (parts.size() >= 3)
        {
            TEScale *scale = (TEScale *)addr;
            scale->Scale.x = parts[0].ToFloat();
            scale->Scale.y = parts[1].ToFloat();
            scale->Scale.z = parts[2].ToFloat();
        }
    }
};

template <> struct TEPropertyDrawer<TETransform>
{
    static bool Draw(void *addr, const TEString &displayName)
    {
        TETransform *transform = (TETransform *)addr;
        bool p = TEPropertyDrawer<TEVector>::Draw(&transform->Position, "Position");
        bool r = TEPropertyDrawer<TERotator>::Draw(&transform->Rotation, "Rotation");
        bool s = TEPropertyDrawer<TEScale>::Draw(&transform->Scale, "Scale");
        return p || r || s;
    }
    static TEString Serialize(void *addr)
    {
        return ""; // Not used directly
    }
    static void Deserialize(void *addr, const TEString &data)
    {
        // Not used directly
    }
};

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
};

template <> struct TEPropertyDrawer<float>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        ImGui::DragFloat(displayName.c_str(), (float *)addr, 0.1f);
    }
};

template <> struct TEPropertyDrawer<int>
{
    static void Draw(void *addr, const std::string &displayName) { ImGui::DragInt(displayName.c_str(), (int *)addr); }
};

template <> struct TEPropertyDrawer<bool>
{
    static void Draw(void *addr, const std::string &displayName) { ImGui::Checkbox(displayName.c_str(), (bool *)addr); }
};

template <> struct TEPropertyDrawer<TEVector2>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec2Control(displayName, *(glm::vec2 *)addr);
    }
};

template <> struct TEPropertyDrawer<glm::vec2>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec2Control(displayName, *(glm::vec2 *)addr);
    }
};

template <> struct TEPropertyDrawer<glm::vec3>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawVec3Control(displayName, *(glm::vec3 *)addr);
    }
};

template <> struct TEPropertyDrawer<TEColor>
{
    static void Draw(void *addr, const std::string &displayName)
    {
        UIUtils::DrawColorControl(displayName, ((TEColor *)addr)->GetValue());
    }
};

// Helper for Enums (requires manual specialization or a common base)
// For now, we can treat them as Int if needed, or provide a specific drawer.

} // namespace TE

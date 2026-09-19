#pragma once
#include "PreRequisites.h"

/// Identifies which UI rendering backend TimeGUI dispatches to at runtime.
/// Configured via Project Settings -> "User Interface".
enum class UIBackendType : uint8_t
{
    DearImGui = 0, ///< Default: Dear ImGui (docking, multi-viewport)
    ForgeUI = 1    ///< Data-oriented ForgeUI (MSDF font, job-system layout)
};

/// Convert a UIBackendType to a human-readable display name.
inline TEString UIBackendTypeToString(UIBackendType type)
{
    switch (type)
    {
    case UIBackendType::DearImGui:
        return "Dear ImGui";
    case UIBackendType::ForgeUI:
        return "ForgeUI";
    default:
        return "Unknown";
    }
}

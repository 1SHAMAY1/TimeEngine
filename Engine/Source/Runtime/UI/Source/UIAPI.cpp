#include "PreRequisites.h"
#include "UIAPI.hpp"
#include "ImGui/ImGuiUIAPI.hpp"

TEScope<UIAPI> UIAPI::Create(UIBackendType type)
{
    switch (type)
    {
    case UIBackendType::DearImGui:
        return CreateScope<ImGuiUIAPI>();
    case UIBackendType::ForgeUI:
        // ForgeUI backend is currently experimental; fallback to default
        return nullptr;
    }
    return nullptr;
}

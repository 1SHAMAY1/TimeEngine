#include "Core/PreRequisites.h"
#include "UI/UIAPI.hpp"
#include "UI/ImGui/ImGuiUIAPI.hpp"
#include "UI/ForgeUI/ForgeUIAPI.hpp"

TEScope<UIAPI> UIAPI::Create(UIBackendType type)
{
    switch (type)
    {
    case UIBackendType::DearImGui:
        return CreateScope<ImGuiUIAPI>();
    case UIBackendType::ForgeUI:
        return CreateScope<ForgeUIAPI>();
    }
    return nullptr;
}

#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"
#include "UI/UIBackendType.hpp"

/// Project Settings page for the UI Engine backend selection.
/// Registered under Category: "User Interface", Name: "UI Engine & Backend".
class TE_API ProjectUISettings : public TEProjectSettings
{
public:
    ProjectUISettings() = default;
    virtual ~ProjectUISettings() override = default;

    virtual TEString GetCategory() const override { return "User Interface"; }
    virtual TEString GetDisplayName() const override { return "UI Engine & Backend"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;

    // Accessor used by UIEngine at startup
    UIBackendType GetSelectedBackend() const { return m_SelectedBackend; }

private:
    UIBackendType m_SelectedBackend = UIBackendType::DearImGui;
    int m_SelectedIndex = 0; // mirrors m_SelectedBackend for combo widget (0 = Dear ImGui, 1 = ForgeUI)
};

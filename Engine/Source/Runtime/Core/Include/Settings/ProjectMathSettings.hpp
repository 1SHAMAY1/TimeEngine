#pragma once
#include "PreRequisites.h"
#include "ProjectSettingsRegistry.hpp"
#include "MathBackendType.hpp"

/// Project Settings page for the Math Engine backend selection.
/// Registered under Category: "Core", Name: "Math Engine & Backend".
class TE_API ProjectMathSettings : public TEProjectSettings
{
public:
    ProjectMathSettings() = default;
    virtual ~ProjectMathSettings() override = default;

    virtual TEString GetCategory() const override { return "Core"; }
    virtual TEString GetDisplayName() const override { return "Math Engine & Backend"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;

    MathBackendType GetSelectedBackend() const { return m_SelectedBackend; }

private:
    MathBackendType m_SelectedBackend = MathBackendType::GLM;
    int m_SelectedIndex = 0; // 0 = GLM, 1 = Custom Math Library
};

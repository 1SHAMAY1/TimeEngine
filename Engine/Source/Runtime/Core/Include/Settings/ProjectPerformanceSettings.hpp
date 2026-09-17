#pragma once
#include "PreRequisites.h"
#include "ProjectSettingsRegistry.hpp"

class TE_API ProjectPerformanceSettings : public TEProjectSettings
{
public:
    ProjectPerformanceSettings() = default;
    virtual ~ProjectPerformanceSettings() override = default;

    virtual TEString GetCategory() const override { return "Engine"; }
    virtual TEString GetDisplayName() const override { return "Performance & Budgets"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

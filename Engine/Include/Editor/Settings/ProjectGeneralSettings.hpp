#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class TE_API ProjectGeneralSettings : public TEProjectSettings
{
public:
    ProjectGeneralSettings() = default;
    virtual ~ProjectGeneralSettings() override = default;

    virtual TEString GetCategory() const override { return "Project"; }
    virtual TEString GetDisplayName() const override { return "General"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

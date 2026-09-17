#pragma once
#include "PreRequisites.h"
#include "ProjectSettingsRegistry.hpp"

class TE_API ProjectRenderingSettings : public TEProjectSettings
{
public:
    ProjectRenderingSettings() = default;
    virtual ~ProjectRenderingSettings() override = default;

    virtual TEString GetCategory() const override { return "Rendering"; }
    virtual TEString GetDisplayName() const override { return "Quality & Display"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class MaterialSystemProjectSettings : public TEProjectSettings
{
public:
    MaterialSystemProjectSettings() = default;
    virtual ~MaterialSystemProjectSettings() override = default;

    virtual TEString GetCategory() const override { return "Rendering"; }
    virtual TEString GetDisplayName() const override { return "Material System"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

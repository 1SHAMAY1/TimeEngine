#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class Lighting2DProjectSettings : public TEProjectSettings
{
public:
    Lighting2DProjectSettings() = default;
    virtual ~Lighting2DProjectSettings() override = default;

    virtual TEString GetCategory() const override { return "Rendering"; }
    virtual TEString GetDisplayName() const override { return "2D Lighting"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

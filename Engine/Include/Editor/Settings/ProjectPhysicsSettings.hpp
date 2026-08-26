#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class TE_API ProjectPhysicsSettings : public TEProjectSettings
{
public:
    ProjectPhysicsSettings() = default;
    virtual ~ProjectPhysicsSettings() override = default;

    virtual TEString GetCategory() const override { return "Physics"; }
    virtual TEString GetDisplayName() const override { return "2D Physics"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

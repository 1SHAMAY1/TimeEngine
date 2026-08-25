#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class IKProjectSettings : public TEProjectSettings
{
public:
    IKProjectSettings() = default;
    virtual ~IKProjectSettings() override = default;

    virtual TEString GetCategory() const override { return "Animation"; }
    virtual TEString GetDisplayName() const override { return "Inverse Kinematics"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

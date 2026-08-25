#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class TE_API ProjectAudioSettings : public TEProjectSettings
{
public:
    ProjectAudioSettings() = default;
    virtual ~ProjectAudioSettings() override = default;

    virtual TEString GetCategory() const override { return "Audio"; }
    virtual TEString GetDisplayName() const override { return "Audio Master"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

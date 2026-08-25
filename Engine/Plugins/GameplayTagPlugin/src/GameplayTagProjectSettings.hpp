#pragma once
#include "Core/PreRequisites.h"
#include "Editor/ProjectSettingsRegistry.hpp"

class GameplayTagProjectSettings : public TEProjectSettings
{
public:
    GameplayTagProjectSettings() = default;
    virtual ~GameplayTagProjectSettings() override = default;

    virtual TEString GetCategory() const override { return "Project"; }
    virtual TEString GetDisplayName() const override { return "Gameplay Tags"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorSettingsRegistry.hpp"

class TE_API EditorAppearanceSettings : public TEEditorSettings
{
public:
    EditorAppearanceSettings() = default;
    virtual ~EditorAppearanceSettings() override = default;

    virtual TEString GetCategory() const override { return "General"; }
    virtual TEString GetDisplayName() const override { return "Appearance & UI"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

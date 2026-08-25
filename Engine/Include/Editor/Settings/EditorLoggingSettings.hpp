#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorSettingsRegistry.hpp"

class TE_API EditorLoggingSettings : public TEEditorSettings
{
public:
    EditorLoggingSettings() = default;
    virtual ~EditorLoggingSettings() override = default;

    virtual TEString GetCategory() const override { return "Tooling"; }
    virtual TEString GetDisplayName() const override { return "Logging"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

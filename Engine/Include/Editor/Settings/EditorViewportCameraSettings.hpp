#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorSettingsRegistry.hpp"

class TE_API EditorViewportCameraSettings : public TEEditorSettings
{
public:
    EditorViewportCameraSettings() = default;
    virtual ~EditorViewportCameraSettings() override = default;

    virtual TEString GetCategory() const override { return "Viewport"; }
    virtual TEString GetDisplayName() const override { return "Camera Navigation"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

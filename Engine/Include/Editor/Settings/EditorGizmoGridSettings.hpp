#pragma once
#include "Core/PreRequisites.h"
#include "Editor/EditorSettingsRegistry.hpp"

class TE_API EditorGizmoGridSettings : public TEEditorSettings
{
public:
    EditorGizmoGridSettings() = default;
    virtual ~EditorGizmoGridSettings() override = default;

    virtual TEString GetCategory() const override { return "Viewport"; }
    virtual TEString GetDisplayName() const override { return "Gizmo & Grid Snapping"; }
    virtual void OnDrawSettingsUI(Ref<EditorLayer> editor) override;
};

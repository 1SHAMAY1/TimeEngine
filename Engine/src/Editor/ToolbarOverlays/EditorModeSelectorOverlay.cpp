#include "Core/PreRequisites.h"
#include "Editor/ToolbarOverlays/EditorModeSelectorOverlay.hpp"
#include "Editor/EditorMode.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(EditorModeSelectorOverlay);

void EditorModeSelectorOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "ModeSelector";
    item.label = "Mode Selector";
    item.alignment = EditorToolbarAlignment::Left;
    item.priority = 0;
    item.width = 200.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("ModeSelector", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void EditorModeSelectorOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
    TEString modeLabel = activeMode ? activeMode->GetName() : "Selection Mode";

    TimeGUI::SetNextItemWidth(200.0f);

    if (TimeGUI::BeginCombo("##ModeSelectorCombo", modeLabel.c_str()))
    {
        for (const auto &mode : EditorModeRegistry::GetModes())
        {
            bool isSelected = (activeMode == mode.get());
            if (TimeGUI::Selectable(mode->GetName(), isSelected))
                EditorModeRegistry::SetActiveMode(mode->GetName());
        }
        TimeGUI::EndCombo();
    }
}

#pragma once

#include "Core/PreRequisites.h"
#include "GameplayTag.hpp"
#include "GameplayTagContainer.hpp"
#include "GameplayTagManager.hpp"
#include "Layers/Layer.hpp"
#include "UI/Widgets/UISearchBar.hpp"
#include "Utils/TimeGUI.hpp"

/**
 * GameplayTagManagerLayer
 * Comprehensive editor layer for managing, inspecting, editing, diagnosing,
 * and importing/exporting gameplay tags in TimeEngine.
 */
class GameplayTagManagerLayer : public Layer
{
public:
    GameplayTagManagerLayer(const TEString &name = "Gameplay Tag Manager");
    virtual ~GameplayTagManagerLayer() override;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

    // Visibility controls
    void SetVisible(bool visible) { m_IsVisible = visible; }
    bool IsVisible() const { return m_IsVisible; }
    void ToggleVisibility() { m_IsVisible = !m_IsVisible; }

    void SetWindowPosition(const TEVector2 &pos) { m_WindowPos = pos; }
    void SetWindowSize(const TEVector2 &size) { m_WindowSize = size; }
    void SetWindowTitle(const TEString &title) { m_WindowTitle = title; }

private:
    void RenderMainWindow();
    void RenderTagHierarchyTree();
    void RenderSelectedTagDetails();
    void RenderAddNewTagSection();
    void RenderDiagnosticsSection();
    void RenderINISection();

    void RefreshDiagnostics();

private:
    bool m_IsVisible = true;
    TEVector2 m_WindowPos = TEVector2(60, 60);
    TEVector2 m_WindowSize = TEVector2(750, 560);
    TEString m_WindowTitle = "Gameplay Tag Manager";

    // TimeEngine UI Search Bar component
    TERef<UISearchBar> m_SearchBar;

    // Selected tag in hierarchy
    GameplayTag m_SelectedTag;

    // Add tag buffer
    TEString m_NewTagNameBuffer;
    TEString m_NewTagDescBuffer;

    // Rename / edit buffer
    TEString m_RenameBuffer;
    TEString m_DescEditBuffer;

    // INI File Path buffer
    TEString m_INIFilePathBuffer = "Config/GameplayTags.ini";
    TEString m_StatusMessage;
    float m_StatusTimer = 0.0f;

    // Diagnostics state
    TEArray<TagValidationIssue> m_CachedIssues;
    bool m_ShowDiagnostics = false;
};

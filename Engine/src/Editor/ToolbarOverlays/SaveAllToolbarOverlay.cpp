#include "Core/PreRequisites.h"
#include "Editor/ToolbarOverlays/SaveAllToolbarOverlay.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/ToolbarOverlays/RestartEditorToolbarOverlay.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(SaveAllToolbarOverlay);

static bool s_ShowSaveModal = false;
static bool s_PendingOpenSaveModal = false;
static bool s_IsAppExitCloseModal = false;
static bool s_IsSavingAndExiting = false;
static bool s_IsRestartPending = false;
static float s_ExitTimer = 0.0f;
static TEMap<TEString, bool> s_SaveSelectionMap;
static std::function<void()> s_OnSaveProceedCallback = nullptr;

void SaveAllToolbarOverlay::OpenSaveModal(bool isAppExit)
{
    s_OnSaveProceedCallback = nullptr;
    auto dirtyList = EditorSaveManager::GetDirtySavables();
    if (!isAppExit && dirtyList.Num() == 1)
    {
        auto &onlySavable = dirtyList[0];
        if (onlySavable && onlySavable->GetSavablePath().IsEmpty())
        {
            onlySavable->Save();
            return;
        }
    }

    s_ShowSaveModal = true;
    s_PendingOpenSaveModal = true;
    s_IsAppExitCloseModal = isAppExit;
    s_SaveSelectionMap.clear();
    for (size_t i = 0; i < dirtyList.Num(); ++i)
    {
        auto &savable = dirtyList[i];
        if (savable)
            s_SaveSelectionMap[savable->GetSavableID()] = true;
    }
}

void SaveAllToolbarOverlay::OpenSaveModalWithAction(std::function<void()> onProceed)
{
    s_OnSaveProceedCallback = onProceed;
    s_ShowSaveModal = true;
    s_PendingOpenSaveModal = true;
    s_IsAppExitCloseModal = false;
    s_SaveSelectionMap.clear();
    auto dirtyList = EditorSaveManager::GetDirtySavables();
    for (size_t i = 0; i < dirtyList.Num(); ++i)
    {
        auto &savable = dirtyList[i];
        if (savable)
            s_SaveSelectionMap[savable->GetSavableID()] = true;
    }
}

bool SaveAllToolbarOverlay::OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor)
{
    if (shortcutId == "Editor_SaveAll")
    {
        OpenSaveModal();
        return true;
    }
    return false;
}

void SaveAllToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    // Register interface-decoupled save prompt callbacks so EditorLayer/Menubar/ContentBrowser
    // can call EditorSaveManager::RequestSavePrompt() without knowing about this toolbar class.
    EditorSaveManager::SetSavePromptCallback([](bool isAppExit) { SaveAllToolbarOverlay::OpenSaveModal(isAppExit); });
    EditorSaveManager::SetSavePromptWithActionCallback([](std::function<void()> action)
                                                       { SaveAllToolbarOverlay::OpenSaveModalWithAction(action); });

    EditorToolbarItem item;
    item.id = "SaveAllButton";
    item.label = "Save All";
    item.alignment = EditorToolbarAlignment::Left;
    item.priority = -10;
    item.width = 36.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("SaveAllButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void SaveAllToolbarOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    bool hasDirty = EditorSaveManager::HasUnsavedChanges();
    size_t dirtyCount = EditorSaveManager::GetDirtyCount();

    float btnWidth = 36.0f;
    float btnHeight = 30.0f;
    TEString btnLabel = "##SaveAllButton";

    if (hasDirty)
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.16f, 0.36f, 0.65f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.22f, 0.46f, 0.80f, 1.0f));
    }
    else
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.13f, 0.16f, 0.20f, 0.90f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.20f, 0.25f, 0.32f, 1.0f));
    }

    TEVector2 btnScreenPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button(btnLabel, TEVector2(btnWidth, btnHeight));

    TimeGUI::PopStyleColor(2);

    float iconSize = 22.0f;
    TEVector2 iconMin(btnScreenPos.x + (btnWidth - iconSize) * 0.5f, btnScreenPos.y + (btnHeight - iconSize) * 0.5f);
    TEVector2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
    EditorUtils::DrawSaveIcon(iconMin, iconMax, hasDirty);

    if (hasDirty && TimeGUI::IsItemHovered())
    {
        TimeGUI::SetTooltip(("Save All Modified Files (" + TEString::FromInt64(static_cast<int64_t>(dirtyCount)) +
                             " unsaved)\nClick to review and save changes.")
                                .c_str());
    }
    else if (!hasDirty && TimeGUI::IsItemHovered())
    {
        TimeGUI::SetTooltip("Save All (All files saved)");
    }

    if (clicked)
        OpenSaveModal();

    if (s_PendingOpenSaveModal)
    {
        TimeGUI::OpenPopup("Save Content");
        s_PendingOpenSaveModal = false;
    }

    TimeGUI::SetNextWindowSize(TEVector2(640.0f, 460.0f), TimeGUI::TimeGUICond_FirstUseEver);
    if (TimeGUI::BeginPopupModal("Save Content", &s_ShowSaveModal, TimeGUI::TimeGUIWindowFlags_NoResize))
    {
        TEVector2 headerIconPos = TimeGUI::GetCursorScreenPos();
        EditorUtils::DrawSaveIcon(TEVector2(headerIconPos.x, headerIconPos.y + 2.0f),
                                  TEVector2(headerIconPos.x + 18.0f, headerIconPos.y + 20.0f), hasDirty);
        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + 24.0f);
        TimeGUI::TextColored(TEColor(0.95f, 0.95f, 0.95f, 1.0f), TEString("Save Content / Modified Files"));
        TimeGUI::TextDisabled("Select the files/assets you wish to save to disk.");
        TimeGUI::Separator();

        auto dirtyList = EditorSaveManager::GetDirtySavables();

        if (TimeGUI::Button("Select All", TEVector2(90.0f, 22.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable)
                    s_SaveSelectionMap[savable->GetSavableID()] = true;
            }
        }
        TimeGUI::SameLine(0, 8);
        if (TimeGUI::Button("Deselect All", TEVector2(90.0f, 22.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable)
                    s_SaveSelectionMap[savable->GetSavableID()] = false;
            }
        }
        TimeGUI::SameLine(0, 15);
        TimeGUI::TextDisabled("%d modified item(s) pending save", (int)dirtyList.Num());
        TimeGUI::Spacing();

        if (TimeGUI::BeginChild("##SaveItemsScrollRegion", TEVector2(0, 260.0f), true,
                                TimeGUI::TimeGUIWindowFlags_AlwaysVerticalScrollbar))
        {
            if (dirtyList.IsEmpty())
            {
                TimeGUI::Spacing();
                TimeGUI::TextDisabled("  No modified or unsaved files found.");
            }
            else
            {
                for (size_t i = 0; i < dirtyList.Num(); ++i)
                {
                    auto &savable = dirtyList[i];
                    if (!savable)
                        continue;

                    TEString id = savable->GetSavableID();
                    bool &isSelected = s_SaveSelectionMap[id];

                    TimeGUI::PushID((int)i);
                    TimeGUI::Checkbox("##Select", &isSelected);
                    TimeGUI::SameLine(0, 8);
                    TimeGUI::TextColored(TEColor(0.95f, 0.70f, 0.20f, 1.0f), TEString("[MODIFIED]"));
                    TimeGUI::SameLine(0, 8);
                    TimeGUI::TextColored(TEColor(1.0f, 1.0f, 1.0f, 1.0f), savable->GetSavableDisplayName());
                    TimeGUI::SameLine(0, 10);
                    TimeGUI::TextDisabled("[%s]", savable->GetSavableType().c_str());

                    TEString pathStr = savable->GetSavablePath();
                    if (pathStr.IsEmpty())
                        pathStr = "(In-Memory Scene / Virtual Asset)";
                    TimeGUI::TextDisabled("    Path: %s", pathStr.c_str());
                    TimeGUI::TextColored(
                        TEColor(0.45f, 0.75f, 0.95f, 0.90f),
                        TEString("    -> Changes: Asset modified in editor session (pending disk write)"));
                    TimeGUI::Separator();
                    TimeGUI::PopID();
                }
            }
        }
        TimeGUI::EndChild();

        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::Spacing();

        if (TimeGUI::Button("Cancel", TEVector2(100.0f, 28.0f)))
        {
            s_ShowSaveModal = false;
            s_IsAppExitCloseModal = false;
            s_IsRestartPending = false;
            s_OnSaveProceedCallback = nullptr;
            TimeGUI::CloseCurrentPopup();
        }

        if (s_IsAppExitCloseModal || s_OnSaveProceedCallback != nullptr)
        {
            TimeGUI::SameLine(0, 8);
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.55f, 0.22f, 0.22f, 0.95f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.70f, 0.28f, 0.28f, 1.0f));
            if (TimeGUI::Button("Don't Save", TEVector2(100.0f, 28.0f)))
            {
                s_ShowSaveModal = false;
                TimeGUI::CloseCurrentPopup();
                if (s_IsAppExitCloseModal)
                {
                    s_IsAppExitCloseModal = false;
                    if (s_IsRestartPending)
                    {
                        s_IsRestartPending = false;
                        RestartEditorToolbarOverlay::RestartEditor();
                    }
                    else
                    {
                        Application::Get().ForceClose();
                    }
                }
                else if (s_OnSaveProceedCallback)
                {
                    auto action = s_OnSaveProceedCallback;
                    s_OnSaveProceedCallback = nullptr;
                    action();
                }
            }
            TimeGUI::PopStyleColor(2);
        }

        TimeGUI::SameLine(0, 12);
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.52f, 0.28f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.22f, 0.62f, 0.35f, 1.0f));
        if (TimeGUI::Button("Save Selected", TEVector2(140.0f, 28.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable && s_SaveSelectionMap[savable->GetSavableID()])
                    savable->Save();
            }
            s_ShowSaveModal = false;
            TimeGUI::CloseCurrentPopup();
            if (s_IsAppExitCloseModal)
            {
                s_IsSavingAndExiting = true;
                s_ExitTimer = 0.0f;
            }
            else if (s_OnSaveProceedCallback)
            {
                auto action = s_OnSaveProceedCallback;
                s_OnSaveProceedCallback = nullptr;
                action();
            }
        }
        TimeGUI::PopStyleColor(2);

        TimeGUI::SameLine(0, 8);
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.40f, 0.75f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.26f, 0.50f, 0.90f, 1.0f));
        if (TimeGUI::Button("Save All", TEVector2(100.0f, 28.0f)))
        {
            EditorSaveManager::SaveAll();
            s_ShowSaveModal = false;
            TimeGUI::CloseCurrentPopup();
            if (s_IsAppExitCloseModal)
            {
                s_IsSavingAndExiting = true;
                s_ExitTimer = 0.0f;
            }
            else if (s_OnSaveProceedCallback)
            {
                auto action = s_OnSaveProceedCallback;
                s_OnSaveProceedCallback = nullptr;
                action();
            }
        }
        TimeGUI::PopStyleColor(2);
        TimeGUI::EndPopup();
    }

    if (s_IsSavingAndExiting)
    {
        s_ExitTimer += TimeGUI::GetIO().DeltaTime;
        TimeGUI::OpenPopup("Shutting Down Modal");
        TimeGUIViewport vp = TimeGUI::GetMainViewport();
        TEVector2 center(vp.Pos.x + vp.Size.x * 0.5f, vp.Pos.y + vp.Size.y * 0.5f);
        TimeGUI::SetNextWindowPos(center, TimeGUI::TimeGUICond_Always, TEVector2(0.5f, 0.5f));
        TimeGUI::SetNextWindowSize(TEVector2(380.0f, 100.0f));
        if (TimeGUI::BeginPopupModal("Shutting Down Modal", nullptr,
                                     TimeGUI::TimeGUIWindowFlags_NoResize | TimeGUI::TimeGUIWindowFlags_NoMove |
                                         TimeGUI::TimeGUIWindowFlags_NoTitleBar))
        {
            TimeGUI::Spacing();
            TimeGUI::TextColored(TEColor(0.25f, 0.85f, 0.45f, 1.0f), s_IsRestartPending
                                                                         ? "  Saving changes & restarting TimeEngine..."
                                                                         : "  Saving changes & closing TimeEngine...");
            TimeGUI::Spacing();
            float progress = (s_ExitTimer < 0.35f) ? (s_ExitTimer / 0.35f) : 1.0f;
            TEVector2 pMin = TimeGUI::GetCursorScreenPos();
            float barW = TimeGUI::GetContentRegionAvail().x;
            float barH = 16.0f;
            TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
            dl.AddRectFilled(pMin, TEVector2(pMin.x + barW, pMin.y + barH), IM_COL32(30, 36, 48, 255), 4.0f);
            dl.AddRectFilled(pMin, TEVector2(pMin.x + barW * progress, pMin.y + barH), IM_COL32(50, 160, 90, 255),
                             4.0f);
            TimeGUI::EndPopup();
        }

        if (s_ExitTimer >= 0.35f)
        {
            s_IsSavingAndExiting = false;
            s_IsAppExitCloseModal = false;
            if (s_IsRestartPending)
            {
                s_IsRestartPending = false;
                RestartEditorToolbarOverlay::RestartEditor();
            }
            else
            {
                Application::Get().ForceClose();
            }
        }
    }

    EditorUtils::DrawFileBrowserModal();
}

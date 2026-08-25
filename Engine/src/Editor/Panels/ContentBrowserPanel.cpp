#include "Core/PreRequisites.h"
#include "Editor/Panels/ContentBrowserPanel.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Project/Project.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorToolbarOverlay.hpp"
#include "Editor/EditorUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "UI/Widgets/UISearchBar.hpp"
#include "UI/Widgets/UITileView.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

ContentBrowserPanel::ContentBrowserPanel()
    : IEditorPanel("Content Browser")
{
    m_SearchBar = CreateRef<UISearchBar>("Search assets...", "##ContentBrowserSearchBar");
    m_TileView = CreateRef<UITileView>(TEVector2(96.0f, 110.0f), "##ContentBrowserTileView");
}

void ContentBrowserPanel::OnAttach()
{
}

TEString ContentBrowserPanel::GetProjectAssetsDirectory() const
{
    return Project::GetAssetDirectory();
}

TEString ContentBrowserPanel::GetEngineResourcesDirectory() const
{
    TEString projDir = Project::GetProjectDirectory();
    if (!projDir.empty())
    {
        TEString relRes = projDir / ".." / ".." / "Resources";
        if (TEFileSystem::Exists(relRes))
            return TEFileSystem::GetAbsolutePath(relRes);
    }

    TEString cwd = TEFileSystem::GetCurrentWorkingDirectory();
    TEString resPath = cwd / "Resources";
    if (TEFileSystem::Exists(resPath))
        return TEFileSystem::GetAbsolutePath(resPath);
    if (TEFileSystem::Exists("Resources"))
        return TEFileSystem::GetAbsolutePath("Resources");
    return resPath;
}

static TEString s_ActiveContentBrowserDir;

TEString ContentBrowserPanel::GetActiveContentBrowserDirectory()
{
    if (!s_ActiveContentBrowserDir.empty() && TEFileSystem::Exists(s_ActiveContentBrowserDir))
        return s_ActiveContentBrowserDir;
    return Project::GetAssetDirectory();
}

void ContentBrowserPanel::NavigateTo(const TEString &newDir, bool recordHistory)
{
    if (newDir.empty() || !TEFileSystem::Exists(newDir))
        return;

    m_CurrentDirectory = newDir;
    m_PathInputBuffer = m_CurrentDirectory;
    s_ActiveContentBrowserDir = m_CurrentDirectory;

    if (recordHistory)
    {
        if (m_HistoryIndex >= 0 && m_HistoryIndex < (int)m_History.Num() - 1)
        {
            size_t itemsToRemove = m_History.Num() - (m_HistoryIndex + 1);
            for (size_t k = 0; k < itemsToRemove; ++k)
            {
                if (!m_History.IsEmpty())
                    m_History.RemoveAt(m_History.Num() - 1);
            }
        }

        if (m_History.IsEmpty() || m_History.Last() != m_CurrentDirectory)
        {
            m_History.Add(m_CurrentDirectory);
            m_HistoryIndex = (int)m_History.Num() - 1;
        }
    }

    if (m_TileView)
        m_TileView->ClearSelection();
    m_SelectedPath = "";
    m_SelectedPaths.Clear();
}

void ContentBrowserPanel::NavigateBack()
{
    if (CanNavigateBack())
    {
        m_HistoryIndex--;
        NavigateTo(m_History[m_HistoryIndex], false);
    }
    else
    {
        TEString rootDir = (m_ActiveRoot == ContentRoot::Assets) ? GetProjectAssetsDirectory() : GetEngineResourcesDirectory();
        if (m_CurrentDirectory != rootDir)
        {
            NavigateTo(m_CurrentDirectory.GetParentPath(), true);
        }
    }
}

void ContentBrowserPanel::NavigateForward()
{
    if (CanNavigateForward())
    {
        m_HistoryIndex++;
        NavigateTo(m_History[m_HistoryIndex], false);
    }
}

void ContentBrowserPanel::SetActiveRoot(ContentRoot root)
{
    m_ActiveRoot = root;
    TEString targetDir = (m_ActiveRoot == ContentRoot::Assets) ? GetProjectAssetsDirectory() : GetEngineResourcesDirectory();
    NavigateTo(targetDir, true);
}

void ContentBrowserPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::Begin(GetTitle().c_str(), &m_Visible);

    m_Focused = TimeGUI::IsWindowFocused(TimeGUIFocusedFlags_RootAndChildWindows);
    m_Hovered = TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_RootAndChildWindows);

    TEString rootDir = (m_ActiveRoot == ContentRoot::Assets) ? GetProjectAssetsDirectory() : GetEngineResourcesDirectory();
    if (m_CurrentDirectory.empty() || !TEFileSystem::Exists(m_CurrentDirectory))
    {
        NavigateTo(rootDir, true);
    }

    // Navigation History Icon Buttons: Back (<) and Forth (>) via EditorUtils
    bool canBack = CanNavigateBack() || (m_CurrentDirectory != rootDir);
    if (EditorUtils::DrawNavIconButton("##NavBack", false, canBack))
    {
        NavigateBack();
    }
    TimeGUI::SameLine(0, 2);

    bool canForth = CanNavigateForward();
    if (EditorUtils::DrawNavIconButton("##NavForward", true, canForth))
    {
        NavigateForward();
    }
    TimeGUI::SameLine(0, 8);

    // Content Root Switcher Buttons (Assets / Resources)
    bool isAssets = (m_ActiveRoot == ContentRoot::Assets);
    if (isAssets)
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.22f, 0.45f, 0.85f, 1.0f));
    if (TimeGUI::Button("Assets"))
    {
        SetActiveRoot(ContentRoot::Assets);
    }
    if (isAssets)
        TimeGUI::PopStyleColor();

    TimeGUI::SameLine(0, 4);

    bool isResources = (m_ActiveRoot == ContentRoot::Resources);
    if (isResources)
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.22f, 0.45f, 0.85f, 1.0f));
    if (TimeGUI::Button("Resources"))
    {
        SetActiveRoot(ContentRoot::Resources);
    }
    if (isResources)
        TimeGUI::PopStyleColor();

    TimeGUI::SameLine(0, 10);

    // Interactive & Editable Path Input Bar
    float pathWidth = TimeGUI::GetWindowWidth() - 440.0f;
    if (pathWidth < 100.0f)
        pathWidth = 100.0f;
    TimeGUI::SetNextItemWidth(pathWidth);

    if (m_PathInputBuffer.empty() || (!m_IsEditingPath && m_PathInputBuffer != m_CurrentDirectory))
    {
        m_PathInputBuffer = m_CurrentDirectory;
    }

    if (TimeGUI::InputTextWithHint("##ContentBrowserPathInput", "Enter directory path...", m_PathInputBuffer,
                                   TimeGUIInputTextFlags_EnterReturnsTrue | TimeGUIInputTextFlags_AutoSelectAll))
    {
        if (TEFileSystem::Exists(m_PathInputBuffer) && TEFileSystem::IsDirectory(m_PathInputBuffer))
        {
            NavigateTo(m_PathInputBuffer, true);
        }
        else
        {
            m_PathInputBuffer = m_CurrentDirectory;
        }
    }
    m_IsEditingPath = TimeGUI::IsItemActive();

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 260.0f);
    if (m_SearchBar)
    {
        m_SearchBar->Draw();
    }

    TimeGUI::Separator();

    // Category Quick Filters
    const char *categories[] = {"All", "Scenes", "Textures", "Materials", "Scripts"};
    for (int i = 0; i < 5; ++i)
    {
        bool isSelected = (m_ActiveCategoryFilter == categories[i]);
        if (isSelected)
            TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEVector4(0.2f, 0.45f, 0.8f, 1.0f));

        if (TimeGUI::Button(categories[i]))
        {
            m_ActiveCategoryFilter = categories[i];
        }

        if (isSelected)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();
    }
    TimeGUI::NewLine();
    TimeGUI::Separator();

    // Directory & Asset Tile View
    TEArray<UITileItem> tileItems;

    if (TEFileSystem::Exists(m_CurrentDirectory))
    {
        auto dirs = TEFileSystem::GetDirectories(m_CurrentDirectory, false);
        for (const auto &dirPath : dirs)
        {
            TEString filename = dirPath.GetFilename();
            if (m_SearchBar && !m_SearchBar->Matches(filename))
                continue;

            UITileItem item;
            item.ID = filename;
            item.Title = filename;
            item.Subtitle = "Folder";
            item.BadgeText = "DIR";
            item.BadgeColor = TEVector4(0.95f, 0.75f, 0.20f, 1.0f);
            item.CustomDrawIcon = [](TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) {
                dl.AddRectFilled(min, max, IM_COL32(230, 175, 45, 230), 4.0f);
                float w = max.x - min.x;
                dl.AddRectFilled(TEVector2(min.x, min.y), TEVector2(min.x + w * 0.45f, min.y + 4.0f),
                                 IM_COL32(255, 205, 75, 255), 2.0f);
            };
            tileItems.push_back(item);
        }

        auto files = TEFileSystem::GetFiles(m_CurrentDirectory, "", false);
        for (const auto &filePath : files)
        {
            TEString filename = filePath.GetFilename();
            TEString ext = filePath.GetExtension();
            TEString stem = filePath.GetStem();

            if (m_SearchBar && !m_SearchBar->Matches(filename))
                continue;

            // Only recognize and show registered/supported engine asset extensions
            auto registeredEditor = AssetEditorRegistry::GetEditorForPath(filePath);
            bool isEngineAsset = (registeredEditor != nullptr) || EditorUtils::IsKnownAssetExtension(filePath);
            if (!isEngineAsset)
                continue;

            if (m_ActiveCategoryFilter == "Scenes" && ext != ".tescene")
                continue;
            if (m_ActiveCategoryFilter == "Textures" && ext != ".tetexture" && ext != ".tesprite" && ext != ".tesheet" && ext != ".tespritesheet")
                continue;
            if (m_ActiveCategoryFilter == "Materials" && ext != ".tematerial" && ext != ".temat" && ext != ".tematinst")
                continue;
            if (m_ActiveCategoryFilter == "Scripts" && ext != ".tscript")
                continue;

            UITileItem item;
            item.ID = filename;
            item.Title = stem;
            item.Subtitle = registeredEditor ? registeredEditor->GetAssetType() : EditorUtils::GetAssetTypeLabel(filePath);
            item.BadgeText = ext.IsEmpty() ? "FILE" : ext.Mid(1);

            if (ext == ".tescene")
            {
                item.BadgeColor = TEVector4(0.25f, 0.55f, 0.90f, 1.0f);
                item.CustomDrawIcon = [filePath](TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) {
                    dl.AddRectFilled(min, max, IM_COL32(40, 110, 220, 230), 4.0f);
                    float cx = (min.x + max.x) * 0.5f;
                    float cy = (min.y + max.y) * 0.5f;
                    dl.AddTriangleFilled(TEVector2(cx - 5.0f, cy - 8.0f), TEVector2(cx + 7.0f, cy),
                                         TEVector2(cx - 5.0f, cy + 8.0f), IM_COL32(255, 255, 255, 255));

                    if (auto savable = EditorSaveManager::FindSavable(filePath))
                    {
                        if (savable->IsDirty())
                        {
                            float dotRadius = 4.0f;
                            TEVector2 dotCenter(max.x - 2.0f, min.y + 2.0f);
                            dl.AddCircleFilled(dotCenter, dotRadius + 1.0f, IM_COL32(0, 0, 0, 220));
                            dl.AddCircleFilled(dotCenter, dotRadius, IM_COL32(255, 175, 25, 255));
                        }
                    }
                };
            }
            else
            {
                item.BadgeColor = registeredEditor
                    ? TEVector4(0.25f, 0.75f, 0.55f, 1.0f)
                    : TEVector4(0.45f, 0.48f, 0.55f, 1.0f);

                item.CustomDrawIcon = [assetType = item.Subtitle, filePath](TimeGUIDrawList &dl, const TEVector2 &min,
                                                                  const TEVector2 &max) {
                    if (!AssetEditorRegistry::DrawAssetIcon(assetType, min, max))
                    {
                        dl.AddRectFilled(min, max, IM_COL32(40, 45, 58, 230), 4.0f);
                        dl.AddRect(min, max, IM_COL32(80, 90, 110, 200), 4.0f, 0, 1.0f);
                    }

                    if (auto savable = EditorSaveManager::FindSavable(filePath))
                    {
                        if (savable->IsDirty())
                        {
                            float dotRadius = 4.0f;
                            TEVector2 dotCenter(max.x - 2.0f, min.y + 2.0f);
                            dl.AddCircleFilled(dotCenter, dotRadius + 1.0f, IM_COL32(0, 0, 0, 220));
                            dl.AddCircleFilled(dotCenter, dotRadius, IM_COL32(255, 175, 25, 255));
                        }
                    }
                };
            }

            tileItems.push_back(item);
        }
    }

    if (m_TileView)
    {
        m_TileView->SetItems(tileItems);
        m_TileView->OnItemClicked = [this](const UITileItem &item) {
            m_SelectedPaths.Clear();
            if (m_TileView)
            {
                for (const auto &id : m_TileView->GetSelectedItems())
                {
                    m_SelectedPaths.Add(m_CurrentDirectory / id);
                }
            }
            m_SelectedPath = m_SelectedPaths.IsEmpty() ? "" : m_SelectedPaths[0];
        };
        m_TileView->OnItemDoubleClicked = [this, editor](const UITileItem &item) {
            TEString fullPath = m_CurrentDirectory / item.ID;
            if (TEFileSystem::IsDirectory(fullPath))
            {
                NavigateTo(fullPath, true);
            }
            else if (fullPath.EndsWith(".tescene"))
            {
                auto loadSceneAction = [editor, fullPath]() {
                    auto newScene = CreateRef<Scene>();
                    SceneSerializer serializer(newScene);
                    if (serializer.Deserialize(fullPath))
                    {
                        newScene->SetName(fullPath.GetStem());
                        newScene->SetAssetPath(fullPath);
                        editor->SetActiveScene(newScene);
                        EditorSaveManager::RegisterSavable(newScene);
                    }
                };

                auto activeScene = editor->GetActiveScene();
                if (activeScene && activeScene->IsDirty())
                {
                    SaveAllToolbarOverlay::OpenSaveModalWithAction(loadSceneAction);
                }
                else
                {
                    loadSceneAction();
                }
            }
            else
            {
                AssetEditorRegistry::OpenAsset(fullPath);
            }
        };
        m_TileView->OnItemContextMenu = [this](const UITileItem &item) {
            if (m_TileView && !m_TileView->IsItemSelected(item.ID))
            {
                m_TileView->SetSelectedItem(item.ID);
            }
            m_SelectedPaths.Clear();
            if (m_TileView)
            {
                for (const auto &id : m_TileView->GetSelectedItems())
                    m_SelectedPaths.Add(m_CurrentDirectory / id);
            }
            m_SelectedPath = m_SelectedPaths.IsEmpty() ? "" : m_SelectedPaths[0];
            TimeGUI::OpenPopup("ContentBrowserContextMenu");
        };
        m_TileView->Draw();
    }

    // Left Click in empty space clears selection
    if (TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_RootAndChildWindows) &&
        TimeGUI::IsMouseClicked(0) && !TimeGUI::IsAnyItemHovered())
    {
        if (m_TileView)
            m_TileView->ClearSelection();
        m_SelectedPath = "";
        m_SelectedPaths.Clear();
    }

    // Right Click in empty space opens context menu
    if (TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_RootAndChildWindows) &&
        TimeGUI::IsMouseClicked(1) && !TimeGUI::IsAnyItemHovered())
    {
        m_SelectedPath = "";
        m_SelectedPaths.Clear();
        if (m_TileView)
            m_TileView->ClearSelection();
        TimeGUI::OpenPopup("ContentBrowserContextMenu");
    }

    // Context Menu for Content Browser (Items & Background)
    if (TimeGUI::BeginPopup("ContentBrowserContextMenu"))
    {
        size_t selectCount = m_TileView ? m_TileView->GetSelectedItems().Num() : 0;
        bool hasSelection = selectCount > 0;
        bool isSingleItem = (selectCount == 1);
        bool isDirectory = isSingleItem && !m_SelectedPath.empty() && TEFileSystem::IsDirectory(m_SelectedPath);

        if (hasSelection)
        {
            if (isSingleItem && TimeGUI::MenuItem("Open"))
            {
                if (isDirectory)
                {
                    NavigateTo(m_SelectedPath, true);
                }
                else if (m_SelectedPath.EndsWith(".tescene"))
                {
                    TEString pathToOpen = m_SelectedPath;
                    auto loadSceneAction = [editor, pathToOpen]() {
                        auto newScene = CreateRef<Scene>();
                        SceneSerializer serializer(newScene);
                        if (serializer.Deserialize(pathToOpen))
                        {
                            newScene->SetName(pathToOpen.GetStem());
                            newScene->SetAssetPath(pathToOpen);
                            editor->SetActiveScene(newScene);
                            EditorSaveManager::RegisterSavable(newScene);
                        }
                    };

                    auto activeScene = editor->GetActiveScene();
                    if (activeScene && activeScene->IsDirty())
                    {
                        SaveAllToolbarOverlay::OpenSaveModalWithAction(loadSceneAction);
                    }
                    else
                    {
                        loadSceneAction();
                    }
                }
                else
                {
                    AssetEditorRegistry::OpenAsset(m_SelectedPath);
                }
            }

            TEString cutLabel = isSingleItem ? "Cut" : ("Cut (" + TEString::FromInt((int)selectCount) + " items)");
            if (TimeGUI::MenuItem(cutLabel, "Ctrl+X"))
            {
                m_ClipboardPaths.Clear();
                if (m_TileView)
                {
                    for (const auto &id : m_TileView->GetSelectedItems())
                        m_ClipboardPaths.Add(m_CurrentDirectory / id);
                }
                m_ClipboardPath = m_ClipboardPaths.IsEmpty() ? "" : m_ClipboardPaths[0];
                m_ClipboardIsCut = true;
            }

            TEString copyLabel = isSingleItem ? "Copy" : ("Copy (" + TEString::FromInt((int)selectCount) + " items)");
            if (TimeGUI::MenuItem(copyLabel, "Ctrl+C"))
            {
                m_ClipboardPaths.Clear();
                if (m_TileView)
                {
                    for (const auto &id : m_TileView->GetSelectedItems())
                        m_ClipboardPaths.Add(m_CurrentDirectory / id);
                }
                m_ClipboardPath = m_ClipboardPaths.IsEmpty() ? "" : m_ClipboardPaths[0];
                m_ClipboardIsCut = false;
            }

            TEString dupLabel = isSingleItem ? "Duplicate" : ("Duplicate (" + TEString::FromInt((int)selectCount) + " items)");
            if (TimeGUI::MenuItem(dupLabel, "Ctrl+D"))
            {
                if (m_TileView)
                {
                    for (const auto &id : m_TileView->GetSelectedItems())
                    {
                        TEString src = m_CurrentDirectory / id;
                        if (TEFileSystem::Exists(src) && !TEFileSystem::IsDirectory(src))
                        {
                            TEString stem = src.GetStem();
                            TEString ext = src.GetExtension();
                            TEString dest;
                            int counter = 1;
                            do
                            {
                                dest = m_CurrentDirectory / (stem + " - Copy" + (counter > 1 ? (" " + TEString::FromInt(counter)) : "") + ext);
                                counter++;
                            } while (TEFileSystem::Exists(dest));

                            TEFileSystem::CopyFile(src, dest, true);
                        }
                    }
                }
            }

            TEString deleteLabel = isSingleItem ? "Delete" : ("Delete (" + TEString::FromInt((int)selectCount) + " items)");
            if (TimeGUI::MenuItem(deleteLabel, "Delete"))
            {
                TEString activeScenePath = (editor && editor->GetActiveScene()) ? editor->GetActiveScene()->GetAssetPath() : "";

                if (m_TileView)
                {
                    for (const auto &id : m_TileView->GetSelectedItems())
                    {
                        TEString target = m_CurrentDirectory / id;
                        if (!activeScenePath.empty() && target.Equals(activeScenePath, ESearchCase::IgnoreCase))
                        {
                            TE_CORE_WARN("Cannot remove the currently opened active scene: '{0}'. Please switch to another scene first.", target);
                            continue;
                        }

                        if (TEFileSystem::Exists(target))
                        {
                            if (TEFileSystem::IsDirectory(target))
                                TEFileSystem::RemoveAll(target);
                            else
                                AssetManager::DeleteAsset(target);
                        }
                    }
                    m_TileView->ClearSelection();
                }
                m_SelectedPath = "";
                m_SelectedPaths.Clear();
            }

            TimeGUI::Separator();
        }

        if (TimeGUI::BeginMenu("Create New"))
        {
            if (TimeGUI::MenuItem("Folder"))
            {
                TEString newFolder = m_CurrentDirectory / "NewFolder";
                int counter = 1;
                while (TEFileSystem::Exists(newFolder))
                {
                    newFolder = m_CurrentDirectory / ("NewFolder (" + TEString::FromInt(counter++) + ")");
                }
                TEFileSystem::CreateDirectory(newFolder);
            }

            TimeGUI::Separator();

            if (TimeGUI::MenuItem("Scene (.tescene)"))
            {
                EditorUtils::FileBrowserConfig config;
                config.Title = "Create Scene";
                config.ActionButtonText = "Create";
                config.DefaultFilename = "NewScene";
                config.FilterExtension = ".tescene";
                config.AllowFilenameInput = true;
                config.InitialDirectory = m_CurrentDirectory;

                EditorUtils::OpenFileBrowser(config, [this, editor](const TEString &chosenPath) {
                    auto newScene = CreateRef<Scene>();
                    newScene->SetName(chosenPath.GetStem());
                    newScene->SetAssetPath(chosenPath);
                    newScene->CreateEntity("Main Camera");
                    SceneSerializer serializer(newScene);
                    serializer.Serialize(chosenPath);
                    TE_CORE_INFO("Created New Scene: {0}", chosenPath);
                });
            }

            if (TimeGUI::MenuItem("Material (.tematerial)"))
            {
                TEString newMatPath = m_CurrentDirectory / "NewMaterial.tematerial";
                int counter = 1;
                while (TEFileSystem::Exists(newMatPath))
                {
                    newMatPath = m_CurrentDirectory / ("NewMaterial (" + TEString::FromInt(counter++) + ").tematerial");
                }
                TEFileSystem::WriteAllText(newMatPath, "{\"Name\":\"NewMaterial\",\"Albedo\":[1.0,1.0,1.0,1.0]}");
            }

            if (TimeGUI::MenuItem("Script (.tscript)"))
            {
                TEString newScriptPath = m_CurrentDirectory / "NewScript.tscript";
                int counter = 1;
                while (TEFileSystem::Exists(newScriptPath))
                {
                    newScriptPath = m_CurrentDirectory / ("NewScript (" + TEString::FromInt(counter++) + ").tscript");
                }
                TEFileSystem::WriteAllText(newScriptPath, "// TimeEngine Script\nfunction OnUpdate(deltaTime) {\n}\n");
            }

            TimeGUI::Separator();

            if (TimeGUI::MenuItem("More Asset Types (Picker)..."))
            {
                AssetEditorRegistry::OpenAssetPicker(m_CurrentDirectory);
            }

            TimeGUI::EndMenu();
        }

        if (TimeGUI::MenuItem("Create Asset (Picker)..."))
        {
            AssetEditorRegistry::OpenAssetPicker(m_CurrentDirectory);
        }

        bool canPaste = !m_ClipboardPaths.IsEmpty() || (!m_ClipboardPath.empty() && TEFileSystem::Exists(m_ClipboardPath));
        if (TimeGUI::MenuItem("Paste", "Ctrl+V", false, canPaste))
        {
            PasteClipboard(m_CurrentDirectory);
        }

        TimeGUI::Separator();

        if (TimeGUI::MenuItem("Select All", "Ctrl+A"))
        {
            if (m_TileView)
                m_TileView->SelectAll();
            m_SelectedPaths.Clear();
            if (m_TileView)
            {
                for (const auto &id : m_TileView->GetSelectedItems())
                    m_SelectedPaths.Add(m_CurrentDirectory / id);
            }
            m_SelectedPath = m_SelectedPaths.IsEmpty() ? "" : m_SelectedPaths[0];
        }

        if (TimeGUI::MenuItem("Show in Explorer"))
        {
            TEString pathToOpen = hasSelection ? m_SelectedPath : m_CurrentDirectory;
            TEString absPath = TEFileSystem::GetAbsolutePath(pathToOpen);
            #if defined(TE_PLATFORM_WINDOWS)
            TEString cmd = "explorer.exe /select,\"" + absPath + "\"";
            system(cmd.c_str());
            #endif
        }

        TimeGUI::EndPopup();
    }

    TimeGUI::End();
}

void ContentBrowserPanel::PasteClipboard(const TEString &targetFolder)
{
    TEArray<TEString> pathsToPaste = m_ClipboardPaths;
    if (pathsToPaste.IsEmpty() && !m_ClipboardPath.empty())
        pathsToPaste.Add(m_ClipboardPath);

    if (pathsToPaste.IsEmpty())
        return;

    for (const auto &srcPath : pathsToPaste)
    {
        if (!TEFileSystem::Exists(srcPath))
            continue;

        TEString filename = srcPath.GetFilename();
        TEString destPath = targetFolder / filename;

        if (destPath == srcPath)
        {
            TEString stem = srcPath.GetStem();
            TEString ext = srcPath.GetExtension();
            int counter = 1;
            do
            {
                destPath = targetFolder / (stem + " - Copy" + (counter > 1 ? (" " + TEString::FromInt(counter)) : "") + ext);
                counter++;
            } while (TEFileSystem::Exists(destPath));
        }

        if (TEFileSystem::IsDirectory(srcPath))
        {
            // Directory move or copy
            if (m_ClipboardIsCut)
            {
                // Move logic if needed
            }
        }
        else
        {
            TEFileSystem::CopyFile(srcPath, destPath, true);
            if (m_ClipboardIsCut)
            {
                AssetManager::DeleteAsset(srcPath);
            }
        }
    }

    if (m_ClipboardIsCut)
    {
        m_ClipboardPaths.Clear();
        m_ClipboardPath = "";
        m_ClipboardIsCut = false;
    }
}

bool ContentBrowserPanel::OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor)
{
    if (!m_Focused)
        return false;

    if (shortcutId == "Editor_SelectAll")
    {
        if (m_TileView)
        {
            m_TileView->SelectAll();
            m_SelectedPaths.Clear();
            for (const auto &id : m_TileView->GetSelectedItems())
                m_SelectedPaths.Add(m_CurrentDirectory / id);
            m_SelectedPath = m_SelectedPaths.IsEmpty() ? "" : m_SelectedPaths[0];
            return true;
        }
    }

    if (shortcutId == "Editor_ClearSelection")
    {
        if (m_TileView)
            m_TileView->ClearSelection();
        m_SelectedPath = "";
        m_SelectedPaths.Clear();
        return true;
    }

    if (shortcutId == "Editor_Copy")
    {
        m_ClipboardPaths.Clear();
        if (m_TileView)
        {
            for (const auto &id : m_TileView->GetSelectedItems())
            {
                TEString p = m_CurrentDirectory / id;
                if (TEFileSystem::Exists(p))
                    m_ClipboardPaths.Add(p);
            }
        }
        if (m_ClipboardPaths.IsEmpty() && !m_SelectedPath.empty() && TEFileSystem::Exists(m_SelectedPath))
            m_ClipboardPaths.Add(m_SelectedPath);

        if (!m_ClipboardPaths.IsEmpty())
        {
            m_ClipboardPath = m_ClipboardPaths[0];
            m_ClipboardIsCut = false;
            return true;
        }
    }

    if (shortcutId == "Editor_Cut")
    {
        m_ClipboardPaths.Clear();
        if (m_TileView)
        {
            for (const auto &id : m_TileView->GetSelectedItems())
            {
                TEString p = m_CurrentDirectory / id;
                if (TEFileSystem::Exists(p))
                    m_ClipboardPaths.Add(p);
            }
        }
        if (m_ClipboardPaths.IsEmpty() && !m_SelectedPath.empty() && TEFileSystem::Exists(m_SelectedPath))
            m_ClipboardPaths.Add(m_SelectedPath);

        if (!m_ClipboardPaths.IsEmpty())
        {
            m_ClipboardPath = m_ClipboardPaths[0];
            m_ClipboardIsCut = true;
            return true;
        }
    }

    if (shortcutId == "Editor_Paste")
    {
        if (!m_ClipboardPaths.IsEmpty() || (!m_ClipboardPath.empty() && TEFileSystem::Exists(m_ClipboardPath)))
        {
            PasteClipboard(m_CurrentDirectory);
            return true;
        }
    }

    if (shortcutId == "Editor_Duplicate")
    {
        TESet<TEString> selectedIDs = m_TileView ? m_TileView->GetSelectedItems() : TESet<TEString>();
        if (selectedIDs.IsEmpty() && !m_SelectedPath.empty())
            selectedIDs.Add(m_SelectedPath.GetFilename());

        for (const auto &id : selectedIDs)
        {
            TEString src = m_CurrentDirectory / id;
            if (TEFileSystem::Exists(src) && !TEFileSystem::IsDirectory(src))
            {
                TEString stem = src.GetStem();
                TEString ext = src.GetExtension();
                TEString dest;
                int counter = 1;
                do
                {
                    dest = m_CurrentDirectory / (stem + " - Copy" + (counter > 1 ? (" " + TEString::FromInt(counter)) : "") + ext);
                    counter++;
                } while (TEFileSystem::Exists(dest));

                TEFileSystem::CopyFile(src, dest, true);
            }
        }
        return !selectedIDs.IsEmpty();
    }

    if (shortcutId == "Editor_DeleteSelected")
    {
        TESet<TEString> selectedIDs = m_TileView ? m_TileView->GetSelectedItems() : TESet<TEString>();
        if (selectedIDs.IsEmpty() && !m_SelectedPath.empty())
            selectedIDs.Add(m_SelectedPath.GetFilename());

        if (selectedIDs.IsEmpty())
            return false;

        TEString activeScenePath = (editor && editor->GetActiveScene()) ? editor->GetActiveScene()->GetAssetPath() : "";

        for (const auto &id : selectedIDs)
        {
            TEString targetPath = m_CurrentDirectory / id;
            if (!activeScenePath.empty() && targetPath.Equals(activeScenePath, ESearchCase::IgnoreCase))
            {
                TE_CORE_WARN("Cannot remove the currently opened active scene: '{0}'. Please switch to another scene first.", targetPath);
                continue;
            }

            if (TEFileSystem::Exists(targetPath))
            {
                if (TEFileSystem::IsDirectory(targetPath))
                    TEFileSystem::RemoveAll(targetPath);
                else
                    AssetManager::DeleteAsset(targetPath);
            }
        }

        if (m_TileView)
            m_TileView->ClearSelection();
        m_SelectedPath = "";
        m_SelectedPaths.Clear();
        return true;
    }

    return false;
}

TE_REGISTER_EDITOR_PANEL(ContentBrowserPanel);


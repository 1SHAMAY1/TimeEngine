#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Project/Project.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/ISavable.hpp"
#include "Editor/Panels/ContentBrowserPanel.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

static void DrawFolderTreeNodes(const TEString &currentDir, TEString &selectedDir)
{
    auto dirs = TEFileSystem::GetDirectories(currentDir);
    for (const auto &subDir : dirs)
    {
        TEString dirName = subDir.GetFilename();
        if (dirName.StartsWith("."))
            continue;

        bool hasSubDirs = !TEFileSystem::GetDirectories(subDir).IsEmpty();
        TimeGUITreeNodeFlags flags = TimeGUITreeNodeFlags_OpenOnArrow | TimeGUITreeNodeFlags_SpanAvailWidth;
        if (!hasSubDirs)
            flags |= TimeGUITreeNodeFlags_Leaf;
        if (selectedDir == subDir)
            flags |= TimeGUITreeNodeFlags_Selected;

        bool nodeOpen = TimeGUI::TreeNodeEx(dirName.c_str(), flags);
        if (TimeGUI::IsItemClicked())
        {
            selectedDir = subDir;
        }

        if (nodeOpen)
        {
            if (hasSubDirs)
                DrawFolderTreeNodes(subDir, selectedDir);
            TimeGUI::TreePop();
        }
    }
}

TEMap<TEString, TERef<AssetEditor>> &AssetEditorRegistry::GetEditorsMap()
{
    static TEMap<TEString, TERef<AssetEditor>> s_Editors;
    return s_Editors;
}

TEArray<EditorTab> &AssetEditorRegistry::GetOpenTabs()
{
    static TEArray<EditorTab> s_OpenTabs;
    return s_OpenTabs;
}

void AssetEditorRegistry::Init()
{
    // Decentralized auto-registration via TE_REGISTER_ASSET_EDITOR in respective asset editor files
}

void AssetEditorRegistry::Register(TERef<AssetEditor> editor)
{
    if (editor)
    {
        GetEditorsMap()[editor->GetAssetType()] = editor;
    }
}

TERef<AssetEditor> AssetEditorRegistry::GetEditor(const TEString &assetType)
{
    auto *found = GetEditorsMap().Find(assetType);
    if (found)
        return *found;
    return nullptr;
}

TERef<AssetEditor> AssetEditorRegistry::GetEditorForPath(const TEString &path)
{
    TEString ext = path.GetExtension();
    for (auto &pair : GetEditorsMap())
    {
        if (!pair.second)
            continue;
        if (pair.second->GetAssetExtension().Equals(ext, ESearchCase::IgnoreCase))
            return pair.second;
        for (const auto &supported : pair.second->GetSupportedExtensions())
        {
            if (supported.Equals(ext, ESearchCase::IgnoreCase))
                return pair.second;
        }
    }
    return nullptr;
}

TEArray<TERef<AssetEditor>> AssetEditorRegistry::GetRegisteredEditors()
{
    TEArray<TERef<AssetEditor>> editors;
    for (auto &pair : GetEditorsMap())
    {
        if (pair.second)
            editors.Add(pair.second);
    }
    return editors;
}

bool AssetEditorRegistry::DrawAssetIcon(const TEString &assetType, const TEVector2 &min, const TEVector2 &max)
{
    auto editor = GetEditor(assetType);
    if (!editor)
        editor = GetEditorForPath(assetType);
    if (editor)
    {
        editor->DrawIcon(min, max);
        return true;
    }
    return false;
}

static TEMap<TEString, TERef<Asset>> s_InMemoryAssetCache;

class AssetTabSavable : public ISavable
{
public:
    AssetTabSavable(const TEString &path, const TEString &type, const TEString &displayName)
        : m_Path(path), m_Type(type), m_DisplayName(displayName), m_IsDirty(false)
    {
    }

    TEString GetSavableID() const override { return m_Path; }
    TEString GetSavableDisplayName() const override { return m_DisplayName.empty() ? m_Path.GetFilename() : m_DisplayName; }
    TEString GetSavableType() const override { return m_Type; }
    TEString GetSavablePath() const override { return m_Path; }
    bool IsDirty() const override { return m_IsDirty; }
    void MarkDirty(bool dirty = true) override { m_IsDirty = dirty; }

    bool Save() override
    {
        auto *cached = s_InMemoryAssetCache.Find(m_Path);
        if (cached && *cached)
        {
            (*cached)->SaveToFile(m_Path);
        }
        m_IsDirty = false;
        return true;
    }

private:
    TEString m_Path;
    TEString m_Type;
    TEString m_DisplayName;
    bool m_IsDirty;
};

void AssetEditorRegistry::OpenAsset(const TEString &assetPath)
{
    auto &openTabs = GetOpenTabs();
    for (size_t i = 0; i < openTabs.Num(); ++i)
    {
        if (openTabs[i].AssetPath == assetPath)
            return;
    }

    auto editor = GetEditorForPath(assetPath);
    TEString assetType = editor ? editor->GetAssetType() : EditorUtils::GetAssetTypeLabel(assetPath);
    TEString filename = assetPath.GetFilename();

    EditorTab tab;
    tab.Title = filename + "###AssetEditor_" + assetPath;
    tab.AssetPath = assetPath;
    tab.Type = assetType;

    // Check if an in-memory modified version is already cached
    auto *cached = s_InMemoryAssetCache.Find(assetPath);
    if (cached && *cached)
    {
        tab.LoadedAsset = *cached;
    }

    auto savable = EditorSaveManager::FindSavable(assetPath);
    if (!savable)
    {
        savable = CreateRef<AssetTabSavable>(assetPath, assetType, filename);
        EditorSaveManager::RegisterSavable(savable);
    }

    openTabs.Add(tab);
}

void AssetEditorRegistry::CloseAsset(const TEString &assetPath)
{
    auto &openTabs = GetOpenTabs();
    for (size_t i = 0; i < openTabs.Num(); ++i)
    {
        if (openTabs[i].AssetPath == assetPath)
        {
            openTabs.RemoveAt(i);
            break;
        }
    }

    // Only unregister and evict from cache if NOT dirty
    if (!IsAssetDirty(assetPath))
    {
        s_InMemoryAssetCache.Remove(assetPath);
        EditorSaveManager::UnregisterSavable(assetPath);
    }
}

void AssetEditorRegistry::MarkAssetDirty(const TEString &assetPath, bool dirty)
{
    auto savable = EditorSaveManager::FindSavable(assetPath);
    if (savable)
    {
        savable->MarkDirty(dirty);
    }
    else
    {
        auto editor = GetEditorForPath(assetPath);
        TEString assetType = editor ? editor->GetAssetType() : EditorUtils::GetAssetTypeLabel(assetPath);
        auto newSavable = CreateRef<AssetTabSavable>(assetPath, assetType, assetPath.GetFilename());
        newSavable->MarkDirty(dirty);
        EditorSaveManager::RegisterSavable(newSavable);
    }

    if (!dirty)
    {
        // Check if tab is currently closed, if so, clean up from cache
        bool isTabOpen = false;
        for (const auto &t : GetOpenTabs())
        {
            if (t.AssetPath == assetPath)
            {
                isTabOpen = true;
                break;
            }
        }
        if (!isTabOpen)
        {
            s_InMemoryAssetCache.Remove(assetPath);
            EditorSaveManager::UnregisterSavable(assetPath);
        }
    }
}

bool AssetEditorRegistry::IsAssetDirty(const TEString &assetPath)
{
    auto savable = EditorSaveManager::FindSavable(assetPath);
    if (savable)
    {
        return savable->IsDirty();
    }
    return false;
}

#include "UI/Widgets/UISearchBar.hpp"

static bool s_ShowAssetPickerModal = false;
static bool s_OpenAssetPickerPopupTriggered = false;
static TEString s_AssetPickerTargetDirectory = "";
static TEString s_AssetPickerNewName = "NewAsset";
static int s_SelectedAssetPickerIndex = 0;
static TERef<UISearchBar> s_AssetPickerSearchBar;

void AssetEditorRegistry::OpenAssetPicker(const TEString &targetDirectory)
{
    s_ShowAssetPickerModal = true;
    s_OpenAssetPickerPopupTriggered = true;
    s_AssetPickerTargetDirectory = targetDirectory;
    s_AssetPickerNewName = "NewAsset";
    s_SelectedAssetPickerIndex = 0;
    if (!s_AssetPickerSearchBar)
        s_AssetPickerSearchBar = CreateRef<UISearchBar>("Search asset types...", "##AssetPickerModalSearch");
    s_AssetPickerSearchBar->Clear();
}

void AssetEditorRegistry::OnTimeGUIRender()
{
    auto &openTabs = GetOpenTabs();
    TEArray<size_t> tabsToClose;

    for (size_t i = 0; i < openTabs.Num(); ++i)
    {
        auto &tab = openTabs[i];
        bool isOpen = true;
        bool isDirty = IsAssetDirty(tab.AssetPath);

        TimeGUI::SetNextWindowSize(TEVector2(900.0f, 600.0f), TimeGUICond_FirstUseEver);
        if (TimeGUI::Begin(tab.Title.c_str(), &isOpen))
        {
            if (TimeGUI::IsWindowFocused(TimeGUIFocusedFlags_RootAndChildWindows))
            {
                if (tab.LoadedAsset)
                {
                    EditorSaveManager::SetActiveSavable(tab.LoadedAsset);
                }
            }

            if (isDirty)
            {
                // Render bright amber dirty badge dot in the corner
                auto dl = TimeGUI::GetWindowDrawList();
                TEVector2 winPos = TimeGUI::GetWindowPos();
                float dotRadius = 4.0f;
                TEVector2 dotCenter(winPos.x + 14.0f, winPos.y + 14.0f);
                dl.AddCircleFilled(dotCenter, dotRadius + 1.0f, IM_COL32(0, 0, 0, 200));
                dl.AddCircleFilled(dotCenter, dotRadius, IM_COL32(255, 175, 25, 255));
            }

            auto editor = GetEditor(tab.Type);
            if (!editor)
                editor = GetEditorForPath(tab.AssetPath);

            if (editor)
            {
                editor->DrawEditor(tab);
                if (tab.LoadedAsset)
                {
                    s_InMemoryAssetCache[tab.AssetPath] = tab.LoadedAsset;
                }
            }
            else
            {
                TimeGUI::TextDisabled("No dedicated Asset Editor registered for extension: %s",
                                      tab.AssetPath.GetExtension().c_str());
            }
        }
        TimeGUI::End();

        if (!isOpen)
            tabsToClose.Add(i);
    }

    for (int i = (int)tabsToClose.Num() - 1; i >= 0; --i)
    {
        CloseAsset(openTabs[tabsToClose[i]].AssetPath);
    }

    // Dynamic Asset Picker Modal (Populated directly from registered Asset Editors)
    if (s_ShowAssetPickerModal)
    {
        if (s_OpenAssetPickerPopupTriggered)
        {
            TimeGUI::OpenPopup("Asset Picker Context Action");
            s_OpenAssetPickerPopupTriggered = false;
        }

        TEVector2 displaySize = TimeGUI::GetIO().DisplaySize;
        TEVector2 centerPos = TEVector2(displaySize.x * 0.5f, displaySize.y * 0.5f);
        TimeGUI::SetNextWindowPos(centerPos, TimeGUICond_Appearing, TEVector2(0.5f, 0.5f));
        TimeGUI::SetNextWindowSize(TEVector2(780.0f, 560.0f), TimeGUICond_Appearing);
        if (TimeGUI::BeginPopupModal("Asset Picker Context Action", &s_ShowAssetPickerModal,
                                     TimeGUIWindowFlags_NoResize))
        {
            auto registeredEditors = GetRegisteredEditors();

            TimeGUI::TextColored(TEVector4(0.25f, 0.75f, 1.0f, 1.0f), "Asset Picker");

            // Editable Target Directory Path
            TimeGUI::Text("Target Folder:");
            TimeGUI::SameLine();
            TimeGUI::SetNextItemWidth(380.0f);
            TimeGUI::InputTextWithHint("##AssetPickerTargetDirInput", "Enter destination folder path...", s_AssetPickerTargetDirectory);

            TimeGUI::SameLine();
            if (TimeGUI::Button("Active Folder", TEVector2(95.0f, 0.0f)))
            {
                s_AssetPickerTargetDirectory = ContentBrowserPanel::GetActiveContentBrowserDirectory();
            }
            if (TimeGUI::IsItemHovered()) TimeGUI::SetTooltip("Use current folder open in Content Browser");

            TimeGUI::SameLine();
            if (TimeGUI::Button("Browse...", TEVector2(75.0f, 0.0f)))
            {
                TimeGUI::OpenPopup("AssetPickerFolderTreePopup");
            }

            TimeGUI::SameLine();
            if (TimeGUI::Button("Assets", TEVector2(65.0f, 0.0f)))
            {
                s_AssetPickerTargetDirectory = Project::GetAssetDirectory();
            }

            // In-Modal Folder Tree Popup
            TimeGUI::SetNextWindowSize(TEVector2(380.0f, 320.0f), TimeGUICond_Appearing);
            if (TimeGUI::BeginPopup("AssetPickerFolderTreePopup"))
            {
                TimeGUI::TextColored(TEVector4(0.35f, 0.70f, 1.0f, 1.0f), "Select Destination Folder");
                TimeGUI::Separator();

                TEString projDir = Project::GetProjectDirectory();
                TEString assetsDir = Project::GetAssetDirectory();

                TimeGUI::BeginChild("##AssetPickerFolderTreeScroll", TEVector2(0, 220.0f), true);

                if (TimeGUI::Selectable(" Project Root", s_AssetPickerTargetDirectory == projDir))
                {
                    s_AssetPickerTargetDirectory = projDir;
                }

                if (TimeGUI::TreeNodeEx(" Assets", TimeGUITreeNodeFlags_DefaultOpen | TimeGUITreeNodeFlags_OpenOnArrow | TimeGUITreeNodeFlags_SpanAvailWidth))
                {
                    if (TimeGUI::IsItemClicked())
                    {
                        s_AssetPickerTargetDirectory = assetsDir;
                    }
                    DrawFolderTreeNodes(assetsDir, s_AssetPickerTargetDirectory);
                    TimeGUI::TreePop();
                }

                TimeGUI::EndChild();
                TimeGUI::Separator();

                if (TimeGUI::Button("Select Folder", TEVector2(120.0f, 0.0f)))
                {
                    TimeGUI::CloseCurrentPopup();
                }
                TimeGUI::SameLine();
                if (TimeGUI::Button("Close", TEVector2(70.0f, 0.0f)))
                {
                    TimeGUI::CloseCurrentPopup();
                }

                TimeGUI::EndPopup();
            }

            TimeGUI::Separator();

            if (s_AssetPickerSearchBar)
            {
                s_AssetPickerSearchBar->Draw();
            }

            TimeGUI::Spacing();

            // Filtered list of registered asset editors
            TimeGUI::BeginChild("##AssetPickerOptionList", TEVector2(0.0f, 320.0f), true);
            TEArray<int> matchingIndices;

            for (size_t i = 0; i < registeredEditors.Num(); ++i)
            {
                const auto &ed = registeredEditors[i];
                if (s_AssetPickerSearchBar &&
                    !s_AssetPickerSearchBar->Matches(ed->GetAssetType()) &&
                    !s_AssetPickerSearchBar->Matches(ed->GetAssetCategory()) &&
                    !s_AssetPickerSearchBar->Matches(ed->GetAssetExtension()))
                {
                    continue;
                }
                matchingIndices.push_back((int)i);
            }

            if (s_SelectedAssetPickerIndex < 0 && !matchingIndices.IsEmpty())
                s_SelectedAssetPickerIndex = matchingIndices[0];

            for (int idx : matchingIndices)
            {
                const auto &ed = registeredEditors[idx];
                bool isSelected = (s_SelectedAssetPickerIndex == idx);

                TimeGUI::PushID(idx);
                TEVector2 rowPos = TimeGUI::GetCursorScreenPos();
                float rowWidth = TimeGUI::GetContentRegionAvail().x;
                float rowHeight = 38.0f;

                if (TimeGUI::Selectable("##AssetPickerItemSelectable", isSelected, 0, TEVector2(rowWidth, rowHeight)))
                {
                    s_SelectedAssetPickerIndex = idx;
                    s_AssetPickerNewName = "New" + ed->GetAssetType();
                }

                // Draw Procedural Icon
                TEVector2 iconMin(rowPos.x + 4.0f, rowPos.y + 4.0f);
                TEVector2 iconMax(iconMin.x + 30.0f, iconMin.y + 30.0f);
                ed->DrawIcon(iconMin, iconMax);

                TimeGUI::SameLine(46.0f);
                TimeGUI::BeginGroup();
                TimeGUI::Text("%s", ed->GetAssetType().c_str());
                TimeGUI::SameLine();
                TimeGUI::TextDisabled("(%s - %s)", ed->GetAssetCategory().c_str(),
                                      ed->GetAssetExtension().empty() ? "Asset" : ed->GetAssetExtension().c_str());
                TimeGUI::TextDisabled("%s", ed->GetAssetDescription().c_str());
                TimeGUI::EndGroup();

                TimeGUI::PopID();
            }

            TimeGUI::EndChild();

            TimeGUI::Separator();
            TimeGUI::Spacing();

            // Name Input & Create Action
            TimeGUI::Text("Asset File Name:");
            TimeGUI::SameLine();
            TimeGUI::SetNextItemWidth(420.0f);
            bool enterPressed = TimeGUI::InputText("##NewAssetFileNameInput", s_AssetPickerNewName,
                                                   TimeGUIInputTextFlags_EnterReturnsTrue);

            TimeGUI::SameLine();
            bool createClicked = TimeGUI::Button("Create Asset", TEVector2(115.0f, 0.0f));

            if (enterPressed || createClicked)
            {
                if (s_SelectedAssetPickerIndex >= 0 && s_SelectedAssetPickerIndex < (int)registeredEditors.Num() &&
                    !s_AssetPickerNewName.empty())
                {
                    const auto &ed = registeredEditors[s_SelectedAssetPickerIndex];
                    TEString ext = ed->GetAssetExtension();
                    TEString finalName = s_AssetPickerNewName;
                    if (!ext.empty() && !finalName.EndsWith(ext))
                    {
                        finalName += ext;
                    }

                    if (!TEFileSystem::Exists(s_AssetPickerTargetDirectory))
                    {
                        TEFileSystem::CreateDirectories(s_AssetPickerTargetDirectory);
                    }

                    TEString fullPath = s_AssetPickerTargetDirectory / finalName;
                    TEString content = ed->CreateDefaultTemplate(s_AssetPickerNewName);

                    TEFileSystem::WriteAllText(fullPath, content);
                    OpenAsset(fullPath);

                    s_ShowAssetPickerModal = false;
                    TimeGUI::CloseCurrentPopup();
                }
            }

            TimeGUI::SameLine();
            if (TimeGUI::Button("Cancel", TEVector2(80.0f, 0.0f)))
            {
                s_ShowAssetPickerModal = false;
                TimeGUI::CloseCurrentPopup();
            }

            TimeGUI::EndPopup();
        }
    }
}

void AssetEditorRegistry::Clear()
{
    auto &openTabs = GetOpenTabs();
    for (auto &tab : openTabs)
    {
        EditorSaveManager::UnregisterSavable(tab.AssetPath);
    }
    GetEditorsMap().Clear();
    GetOpenTabs().Clear();
    s_AssetPickerSearchBar.reset();
}


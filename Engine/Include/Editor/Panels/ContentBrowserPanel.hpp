#pragma once
#include "Editor/AssetEditor.hpp"
#include "Editor/Panels/IEditorPanel.hpp"
#include <memory>

class TE_API ContentBrowserPanel : public IEditorPanel
{
public:
    enum class ContentRoot
    {
        Assets,
        Resources
    };

    ContentBrowserPanel();
    TEString GetID() const override { return "ContentBrowser"; }
    TEString GetTitle() const override { return "Content Browser"; }
    void OnAttach() override;
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;
    bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) override;

    ContentRoot GetActiveRoot() const { return m_ActiveRoot; }
    void SetActiveRoot(ContentRoot root);
    TEString GetEngineResourcesDirectory() const;
    TEString GetProjectAssetsDirectory() const;
    const TEString &GetCurrentDirectory() const { return m_CurrentDirectory; }
    static TEString GetActiveContentBrowserDirectory();

    void NavigateTo(const TEString &newDir, bool recordHistory = true);
    bool CanNavigateBack() const { return m_HistoryIndex > 0; }
    bool CanNavigateForward() const { return m_HistoryIndex >= 0 && m_HistoryIndex + 1 < (int)m_History.Num(); }
    void NavigateBack();
    void NavigateForward();

private:
    void PasteClipboard(const TEString &targetFolder);

    TERef<class UISearchBar> m_SearchBar;
    TERef<class UITileView> m_TileView;

    ContentRoot m_ActiveRoot = ContentRoot::Assets;
    TEString m_CurrentDirectory;
    TEString m_PathInputBuffer;
    bool m_IsEditingPath = false;
    TEArray<TEString> m_History;
    int m_HistoryIndex = -1;
    TEString m_SelectedPath;
    TEArray<TEString> m_SelectedPaths;
    TEString m_RenamingPath;
    TEString m_ClipboardPath;
    TEArray<TEString> m_ClipboardPaths;
    bool m_ClipboardIsCut = false;
    TEString m_ActiveCategoryFilter = "All";

    TEArray<struct EditorTab> m_OpenEditorTabs;
    int m_ActiveTabRequest = -1;
    TERef<class Asset> m_SelectedBrowserAsset;

    bool m_Focused = false;
    bool m_Hovered = false;

    TERef<class Texture> m_FileIcon;
    TERef<class Texture> m_FolderIcon;
    TERef<class Texture> m_LeftArrowIcon;
};

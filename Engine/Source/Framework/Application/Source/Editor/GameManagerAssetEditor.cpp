#include "PreRequisites.h"
#include "GameManagerAssetEditor.hpp"
#include "AssetEditorRegistry.hpp"
#include "Project/Project.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

TE_REGISTER_ASSET_EDITOR(GameManagerAssetEditor)

GameManagerAssetEditor::GameManagerAssetEditor()
    : m_CodeEdit("##GMCodeEdit")
{
    m_CodeEdit.SetLanguage(ECodeLanguage::TScript);
}

void GameManagerAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float pad = w * 0.12f;

    // Card background in Warm Amber / Gold
    dl.AddRectFilled(min, max, IM_COL32(225, 160, 30, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(32, 28, 22, 255), 2.0f);

    // Geometric Crown / Command Crest
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;

    // Base bar of crown
    dl.AddRectFilled(TEVector2(cx - 10.0f, cy + 2.0f), TEVector2(cx + 10.0f, cy + 6.0f),
                     IM_COL32(245, 195, 45, 255), 1.0f);

    // Crown peaks (3 triangles)
    dl.AddTriangleFilled(TEVector2(cx - 10.0f, cy + 2.0f), TEVector2(cx - 9.0f, cy - 6.0f),
                         TEVector2(cx - 3.0f, cy + 2.0f), IM_COL32(245, 195, 45, 255));
    dl.AddTriangleFilled(TEVector2(cx - 4.5f, cy + 2.0f), TEVector2(cx, cy - 9.0f),
                         TEVector2(cx + 4.5f, cy + 2.0f), IM_COL32(255, 225, 90, 255));
    dl.AddTriangleFilled(TEVector2(cx + 3.0f, cy + 2.0f), TEVector2(cx + 9.0f, cy - 6.0f),
                         TEVector2(cx + 10.0f, cy + 2.0f), IM_COL32(245, 195, 45, 255));
}

void GameManagerAssetEditor::DrawEditor(EditorTab &tab)
{
    auto gmAsset = std::dynamic_pointer_cast<GameManagerAsset>(tab.LoadedAsset);
    if (!gmAsset)
    {
        gmAsset = CreateRef<GameManagerAsset>();
        gmAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = gmAsset;
    }
    if (!gmAsset)
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Failed to load GameManager asset.");
        return;
    }

    if (m_LastLoadedPath != tab.AssetPath)
    {
        m_CodeEdit.SetText(gmAsset->GetSourceText());
        m_LastLoadedPath = tab.AssetPath;

        m_CodeEdit.OnTextChanged = [&tab, gmAsset](const TEString &newText)
        {
            gmAsset->SetSourceText(newText);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        };
    }

    // Top Header & Save Button
    TimeGUI::TextColored(TEColor(0.85f, 0.88f, 0.92f, 1.0f), "Game Manager: %s", gmAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%s)", tab.AssetPath.c_str());

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 110.0f);
    if (TimeGUI::Button("Save Asset", TEVector2(95.0f, 24.0f)))
    {
        gmAsset->SetSourceText(m_CodeEdit.GetText());
        gmAsset->SaveToFile(tab.AssetPath);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
    }

    TimeGUI::Separator();

    // Linkage Section
    TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.2f, 1.0f), "Gameplay Bindings");

    // Player Asset Dropdown Picker
    TEString currentPlayer = gmAsset->GetPlayerAssetPath();
    TEString playerPreview = currentPlayer.empty() ? "[None]" : currentPlayer.GetFilename();

    if (TimeGUI::BeginCombo("Default Player Asset (.teplayer)", playerPreview.c_str()))
    {
        if (TimeGUI::Selectable("[None]", currentPlayer.empty()))
        {
            gmAsset->SetPlayerAssetPath("");
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty() && TEFileSystem::Exists(assetDir))
        {
            TEArray<TEString> playerFiles = TEFileSystem::GetFiles(assetDir, ".teplayer", true);
            for (const auto &filePath : playerFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentPlayer == filePath || currentPlayer == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    gmAsset->SetPlayerAssetPath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
        }
        TimeGUI::EndCombo();
    }

    // Controller Asset Dropdown Picker
    TEString currentController = gmAsset->GetControllerAssetPath();
    TEString controllerPreview = currentController.empty() ? "[None]" : currentController.GetFilename();

    if (TimeGUI::BeginCombo("Default Controller Asset (.tecontroller)", controllerPreview.c_str()))
    {
        if (TimeGUI::Selectable("[None]", currentController.empty()))
        {
            gmAsset->SetControllerAssetPath("");
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty() && TEFileSystem::Exists(assetDir))
        {
            TEArray<TEString> controllerFiles = TEFileSystem::GetFiles(assetDir, ".tecontroller", true);
            for (const auto &filePath : controllerFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentController == filePath || currentController == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    gmAsset->SetControllerAssetPath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
        }
        TimeGUI::EndCombo();
    }

    TimeGUI::Separator();

    // Embedded TScript Code Editor
    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "Game Manager Script Logic (TScript)");
    m_CodeEdit.DrawSelf();
}

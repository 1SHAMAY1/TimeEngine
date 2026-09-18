#include "PreRequisites.h"
#include "PlayerControllerAssetEditor.hpp"
#include "AssetEditorRegistry.hpp"
#include "Project/Project.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

TE_REGISTER_ASSET_EDITOR(PlayerControllerAssetEditor)

PlayerControllerAssetEditor::PlayerControllerAssetEditor()
    : m_CodeEdit("##PCCodeEdit")
{
    m_CodeEdit.SetLanguage(ECodeLanguage::TScript);
}

void PlayerControllerAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float pad = w * 0.12f;

    // Card background in Vibrant Purple
    dl.AddRectFilled(min, max, IM_COL32(165, 80, 220, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(28, 20, 36, 255), 2.0f);

    // Gamepad Silhouette
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;

    // Gamepad body
    dl.AddRectFilled(TEVector2(cx - 13.0f, cy - 7.5f), TEVector2(cx + 13.0f, cy + 7.5f),
                     IM_COL32(200, 130, 255, 255), 3.5f);

    // Left D-Pad cross
    float dx = cx - 6.5f;
    float ds = 3.5f;
    dl.AddRectFilled(TEVector2(dx - 1.2f, cy - ds), TEVector2(dx + 1.2f, cy + ds),
                     IM_COL32(28, 20, 36, 255), 0.5f);
    dl.AddRectFilled(TEVector2(dx - ds, cy - 1.2f), TEVector2(dx + ds, cy + 1.2f),
                     IM_COL32(28, 20, 36, 255), 0.5f);

    // Right Action Buttons (4 dots)
    float bx = cx + 6.5f;
    float br = 1.2f;
    dl.AddCircleFilled(TEVector2(bx, cy - 2.8f), br, IM_COL32(255, 120, 120, 255));
    dl.AddCircleFilled(TEVector2(bx, cy + 2.8f), br, IM_COL32(120, 255, 120, 255));
    dl.AddCircleFilled(TEVector2(bx - 2.8f, cy), br, IM_COL32(120, 180, 255, 255));
    dl.AddCircleFilled(TEVector2(bx + 2.8f, cy), br, IM_COL32(255, 220, 100, 255));
}

void PlayerControllerAssetEditor::DrawEditor(EditorTab &tab)
{
    auto pcAsset = std::dynamic_pointer_cast<PlayerControllerAsset>(tab.LoadedAsset);
    if (!pcAsset)
    {
        pcAsset = CreateRef<PlayerControllerAsset>();
        pcAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = pcAsset;
    }
    if (!pcAsset)
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Failed to load PlayerController asset.");
        return;
    }

    if (m_LastLoadedPath != tab.AssetPath)
    {
        m_CodeEdit.SetText(pcAsset->GetSourceText());
        m_LastLoadedPath = tab.AssetPath;

        m_CodeEdit.OnTextChanged = [&tab, pcAsset](const TEString &newText)
        {
            pcAsset->SetSourceText(newText);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        };
    }

    // Top Header & Save Button
    TimeGUI::TextColored(TEColor(0.85f, 0.88f, 0.92f, 1.0f), "Player Controller: %s", pcAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%s)", tab.AssetPath.c_str());

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 110.0f);
    if (TimeGUI::Button("Save Asset", TEVector2(95.0f, 24.0f)))
    {
        pcAsset->SetSourceText(m_CodeEdit.GetText());
        pcAsset->SaveToFile(tab.AssetPath);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
    }

    TimeGUI::Separator();

    // Input Mapping Context & Settings
    TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.2f, 1.0f), "Input Mapping Context & Controls");

    // IMC Dropdown Combo
    TEString currentImc = pcAsset->GetMappingContextPath();
    TEString imcPreview = currentImc.empty() ? "[None]" : currentImc.GetFilename();

    if (TimeGUI::BeginCombo("Input Mapping Context (.teimc)", imcPreview.c_str()))
    {
        if (TimeGUI::Selectable("[None]", currentImc.empty()))
        {
            pcAsset->SetMappingContextPath("");
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty() && TEFileSystem::Exists(assetDir))
        {
            TEArray<TEString> imcFiles = TEFileSystem::GetFiles(assetDir, ".teimc", true);
            for (const auto &filePath : imcFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentImc == filePath || currentImc == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    pcAsset->SetMappingContextPath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
        }
        TimeGUI::EndCombo();
    }

    bool lookAtCursor = pcAsset->GetEnableLookAtCursor();
    if (TimeGUI::Checkbox("Rotate Towards Mouse Cursor (Look At Cursor)", &lookAtCursor))
    {
        pcAsset->SetEnableLookAtCursor(lookAtCursor);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    float deadzone = pcAsset->GetDeadzone();
    if (TimeGUI::SliderFloat("Analog Stick Deadzone", &deadzone, 0.0f, 0.5f, "%.2f"))
    {
        pcAsset->SetDeadzone(deadzone);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Separator();

    // Embedded TScript Code Editor
    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "Player Controller Script Logic (TScript)");
    m_CodeEdit.DrawSelf();
}

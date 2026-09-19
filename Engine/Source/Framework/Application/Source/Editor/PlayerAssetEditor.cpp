#include "PreRequisites.h"
#include "PlayerAssetEditor.hpp"
#include "AssetEditorRegistry.hpp"
#include "Project/Project.hpp"
#include "Texture.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

TE_REGISTER_ASSET_EDITOR(PlayerAssetEditor)

PlayerAssetEditor::PlayerAssetEditor() : m_CodeEdit("##PlayerCodeEdit")
{
    m_CodeEdit.SetLanguage(ECodeLanguage::TScript);
}

void PlayerAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float pad = w * 0.12f;

    // Card background in Emerald / Cyan
    dl.AddRectFilled(min, max, IM_COL32(35, 185, 135, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(20, 32, 28, 255), 2.0f);

    // Character Silhouette (Head + Shoulders)
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;

    // Head circle
    dl.AddCircleFilled(TEVector2(cx, cy - 5.0f), 6.0f, IM_COL32(75, 225, 175, 255));

    // Torso / Shoulders
    dl.AddRectFilled(TEVector2(cx - 10.0f, cy + 3.0f), TEVector2(cx + 10.0f, cy + 10.0f), IM_COL32(50, 195, 150, 255),
                     3.5f);
}

void PlayerAssetEditor::DrawEditor(EditorTab &tab)
{
    auto playerAsset = std::dynamic_pointer_cast<PlayerAsset>(tab.LoadedAsset);
    if (!playerAsset)
    {
        playerAsset = CreateRef<PlayerAsset>();
        playerAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = playerAsset;
    }
    if (!playerAsset)
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Failed to load Player asset.");
        return;
    }

    if (m_LastLoadedPath != tab.AssetPath)
    {
        m_CodeEdit.SetText(playerAsset->GetSourceText());
        m_LastLoadedPath = tab.AssetPath;

        m_CodeEdit.OnTextChanged = [&tab, playerAsset](const TEString &newText)
        {
            playerAsset->SetSourceText(newText);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        };
    }

    // Top Header & Save Button
    TimeGUI::TextColored(TEColor(0.85f, 0.88f, 0.92f, 1.0f), "Player Character: %s", playerAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%s)", tab.AssetPath.c_str());

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 110.0f);
    if (TimeGUI::Button("Save Asset", TEVector2(95.0f, 24.0f)))
    {
        playerAsset->SetSourceText(m_CodeEdit.GetText());
        playerAsset->SaveToFile(tab.AssetPath);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
    }

    TimeGUI::Separator();

    // Split Columns: Left = Parameters & Code, Right = 2D Preview Canvas
    TimeGUI::Columns(2, "PlayerSplitColumns", true);

    // Left Column
    TimeGUI::BeginChild("PlayerLeftPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.2f, 1.0f), "Appearance & Sprite");

    // Sprite Dropdown Combo
    TEString currentSprite = playerAsset->GetSpritePath();
    TEString spritePreview = currentSprite.empty() ? "[None]" : currentSprite.GetFilename();

    if (TimeGUI::BeginCombo("Sprite / Texture (.tesprite, .png)", spritePreview.c_str()))
    {
        if (TimeGUI::Selectable("[None]", currentSprite.empty()))
        {
            playerAsset->SetSpritePath("");
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty() && TEFileSystem::Exists(assetDir))
        {
            TEArray<TEString> spriteFiles = TEFileSystem::GetFiles(assetDir, ".tesprite", true);
            TEArray<TEString> pngFiles = TEFileSystem::GetFiles(assetDir, ".png", true);
            TEArray<TEString> jpgFiles = TEFileSystem::GetFiles(assetDir, ".jpg", true);

            for (const auto &filePath : spriteFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentSprite == filePath || currentSprite == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    playerAsset->SetSpritePath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
            for (const auto &filePath : pngFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentSprite == filePath || currentSprite == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    playerAsset->SetSpritePath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
            for (const auto &filePath : jpgFiles)
            {
                TEString filename = filePath.GetFilename();
                bool isSelected = (currentSprite == filePath || currentSprite == filename);
                if (TimeGUI::Selectable(filename.c_str(), isSelected))
                {
                    playerAsset->SetSpritePath(filePath);
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                }
            }
        }
        TimeGUI::EndCombo();
    }

    TimeGUI::Separator();
    TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.2f, 1.0f), "Movement Physics");

    int modeIdx = (playerAsset->GetMovementMode() == EPlayerAssetMovementMode::SideScroller) ? 1 : 0;
    const char *modeNames[] = {"Top-Down (4-Way / 8-Way)", "Side-Scroller (Platformer Run & Jump)"};
    if (TimeGUI::Combo("Movement Mode", &modeIdx, modeNames, 2))
    {
        playerAsset->SetMovementMode((modeIdx == 1) ? EPlayerAssetMovementMode::SideScroller
                                                    : EPlayerAssetMovementMode::TopDown);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    float maxSpeed = playerAsset->GetMaxSpeed();
    if (TimeGUI::SliderFloat("Max Speed", &maxSpeed, 1.0f, 50.0f, "%.1f u/s"))
    {
        playerAsset->SetMaxSpeed(maxSpeed);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    float acceleration = playerAsset->GetAcceleration();
    if (TimeGUI::SliderFloat("Acceleration", &acceleration, 1.0f, 100.0f, "%.1f"))
    {
        playerAsset->SetAcceleration(acceleration);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    float friction = playerAsset->GetFriction();
    if (TimeGUI::SliderFloat("Friction", &friction, 0.0f, 50.0f, "%.1f"))
    {
        playerAsset->SetFriction(friction);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    if (playerAsset->GetMovementMode() == EPlayerAssetMovementMode::SideScroller)
    {
        float jumpForce = playerAsset->GetJumpForce();
        if (TimeGUI::SliderFloat("Jump Force", &jumpForce, 1.0f, 50.0f, "%.1f"))
        {
            playerAsset->SetJumpForce(jumpForce);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TEVector2 colliderSize = playerAsset->GetColliderSize();
    if (TimeGUI::DragFloat2("Collider Size (W, H)", &colliderSize.x, 0.05f, 0.1f, 10.0f))
    {
        playerAsset->SetColliderSize(colliderSize);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Separator();
    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "Player Script Logic (TScript)");
    m_CodeEdit.DrawSelf();

    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // Right Column: 2D Preview Canvas
    TimeGUI::BeginChild("PlayerPreviewPanel", TEVector2(0.0f, 0.0f), false);
    TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.2f, 1.0f), "2D Preview Canvas");
    TimeGUI::Separator();

    static float s_Zoom = 1.0f;
    TimeGUI::SliderFloat("Preview Zoom", &s_Zoom, 0.5f, 4.0f, "%.1fx");

    TimeGUI::Spacing();
    TimeGUI::Text("Collider Bounds: (%.2f x %.2f)", colliderSize.x, colliderSize.y);
    TimeGUI::Text("Mode: %s", (playerAsset->GetMovementMode() == EPlayerAssetMovementMode::SideScroller)
                                  ? "Side-Scroller"
                                  : "Top-Down");

    // Interactive canvas box
    TimeGUI::BeginChild("CanvasBox", TEVector2(0.0f, 260.0f), true);
    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "[ Character Preview ]");
    TimeGUI::TextDisabled("Sprite: %s", playerAsset->GetSpritePath().empty() ? "(No Sprite Assigned - Default Quad)"
                                                                             : playerAsset->GetSpritePath().c_str());
    TimeGUI::EndChild();

    TimeGUI::EndChild();

    TimeGUI::Columns(1);
}

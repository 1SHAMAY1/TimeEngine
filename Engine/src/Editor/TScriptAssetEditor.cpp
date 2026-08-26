#include "Editor/TScriptAssetEditor.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scripting/TScriptAsset.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

void TScriptAssetEditor::DrawEditor(EditorTab &tab)
{
    auto scriptAsset = std::dynamic_pointer_cast<TScriptAsset>(tab.LoadedAsset);
    if (!scriptAsset)
    {
        scriptAsset = CreateRef<TScriptAsset>();
        scriptAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = scriptAsset;
    }
    if (!scriptAsset)
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Error: Failed to load TScript asset.");
        return;
    }

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "TScript Editor: %s", scriptAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%s)", tab.AssetPath.c_str());

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 180.0f);
    if (TimeGUI::Button("Save Script", TEVector2(80.0f, 24.0f)))
    {
        if (scriptAsset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Recompile", TEVector2(80.0f, 24.0f)))
    {
        scriptAsset->Recompile();
    }

    TimeGUI::Separator();

    // Check active hooks in the AST
    bool hasReady = false;
    bool hasUpdate = false;
    bool hasCollision = false;
    bool hasInput = false;
    bool hasTimer = false;
    bool hasDestroy = false;

    auto checkEvent = [&](const TEString &name)
    {
        if (name == "on_ready")
            hasReady = true;
        else if (name == "on_update")
            hasUpdate = true;
        else if (name == "on_collision")
            hasCollision = true;
        else if (name == "on_input")
            hasInput = true;
        else if (name == "on_timer")
            hasTimer = true;
        else if (name == "on_destroy")
            hasDestroy = true;
    };

    for (const auto &cls : scriptAsset->CachedAST.classes)
    {
        for (const auto &member : cls->members)
        {
            if (member->type == ASTNodeType::EventFunc)
            {
                auto evt = std::static_pointer_cast<EventFuncNode>(member);
                checkEvent(evt->eventName);
            }
        }
    }

    for (const auto &stmt : scriptAsset->CachedAST.topLevel)
    {
        if (stmt->type == ASTNodeType::EventFunc)
        {
            auto evt = std::static_pointer_cast<EventFuncNode>(stmt);
            checkEvent(evt->eventName);
        }
    }

    // Status line & Hook Badges
    if (scriptAsset->ASTValid)
    {
        TimeGUI::TextColored(TEColor(0.2f, 1.0f, 0.3f, 1.0f), "Status: Compiled OK");
    }
    else
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Status: Error: %s", scriptAsset->CompileError.c_str());
    }

    TimeGUI::SameLine();
    TimeGUI::TextDisabled("| Hooks:");
    TimeGUI::SameLine();
    TimeGUI::TextColored(hasReady ? TEColor(0.2f, 1.0f, 0.3f, 1.0f) : TEColor(0.5f, 0.5f, 0.5f, 1.0f), "on_ready %s",
                         hasReady ? "[OK]" : "[-]");
    TimeGUI::SameLine();
    TimeGUI::TextColored(hasUpdate ? TEColor(0.2f, 1.0f, 0.3f, 1.0f) : TEColor(0.5f, 0.5f, 0.5f, 1.0f), "on_update %s",
                         hasUpdate ? "[OK]" : "[-]");
    TimeGUI::SameLine();
    TimeGUI::TextColored(hasCollision ? TEColor(0.2f, 1.0f, 0.3f, 1.0f) : TEColor(0.5f, 0.5f, 0.5f, 1.0f),
                         "on_collision %s", hasCollision ? "[OK]" : "[-]");
    TimeGUI::SameLine();
    TimeGUI::TextColored(hasInput ? TEColor(0.2f, 1.0f, 0.3f, 1.0f) : TEColor(0.5f, 0.5f, 0.5f, 1.0f), "on_input %s",
                         hasInput ? "[OK]" : "[-]");

    TimeGUI::Separator();

    // Source code editor
    if (TimeGUI::InputTextMultiline("##ScriptSource", scriptAsset->SourceText, TEVector2(-1.0f, -1.0f)))
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
}

void TScriptAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Purple / Violet
    dl.AddRectFilled(min, max, IM_COL32(140, 60, 220, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(28, 22, 40, 255), 2.0f);

    // Stylized Code Brackets '< / >' or '{ ; }'
    float cy = (iMin.y + iMax.y) * 0.5f;
    float cx = (iMin.x + iMax.x) * 0.5f;
    dl.AddLine(TEVector2(cx - 8.0f, cy - 6.0f), TEVector2(cx - 12.0f, cy), IM_COL32(200, 150, 255, 255), 1.5f);
    dl.AddLine(TEVector2(cx - 12.0f, cy), TEVector2(cx - 8.0f, cy + 6.0f), IM_COL32(200, 150, 255, 255), 1.5f);
    dl.AddLine(TEVector2(cx + 8.0f, cy - 6.0f), TEVector2(cx + 12.0f, cy), IM_COL32(200, 150, 255, 255), 1.5f);
    dl.AddLine(TEVector2(cx + 12.0f, cy), TEVector2(cx + 8.0f, cy + 6.0f), IM_COL32(200, 150, 255, 255), 1.5f);
    dl.AddLine(TEVector2(cx + 4.0f, cy - 8.0f), TEVector2(cx - 4.0f, cy + 8.0f), IM_COL32(120, 230, 200, 255), 1.5f);
}

TE_REGISTER_ASSET_EDITOR(TScriptAssetEditor);

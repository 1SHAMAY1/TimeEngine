#include "PreRequisites.h"
#include "TScriptAssetEditor.hpp"
#include "TScriptAsset.hpp"
#include "AssetEditorRegistry.hpp"
#include "TScriptAutoComplete.hpp"
#include "TScriptSyntaxHighlighter.hpp"
#include "TimeGUI.hpp"

TScriptAssetEditor::TScriptAssetEditor() : m_CodeEdit("##TScriptCodeEdit")
{
    m_CodeEdit.SetLanguage(ECodeLanguage::TScript);
}

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

    // Sync loaded content into UICodeEdit if newly opened
    if (m_LastLoadedPath != tab.AssetPath)
    {
        m_CodeEdit.SetText(scriptAsset->SourceText);
        m_LastLoadedPath = tab.AssetPath;

        m_CodeEdit.OnTextChanged = [&tab, scriptAsset](const TEString &newText)
        {
            scriptAsset->SourceText = newText;
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            scriptAsset->Recompile();
        };
    }

    // Pass compiler diagnostics to UICodeEdit
    if (!scriptAsset->ASTValid && scriptAsset->ErrorLine > 0)
    {
        m_CodeEdit.SetErrorMarker(scriptAsset->ErrorLine, scriptAsset->CompileError);
    }
    else
    {
        m_CodeEdit.ClearErrorMarkers();
    }

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "TScript Editor: %s", scriptAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(%s)", tab.AssetPath.c_str());

    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 360.0f);
    if (TimeGUI::Button("Find (Ctrl+F)", TEVector2(85.0f, 24.0f)))
    {
        m_CodeEdit.ToggleSearchBar();
    }
    TimeGUI::SameLine();
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
    TimeGUI::SameLine();
    if (TimeGUI::Button(m_ShowAPIBrowser ? "APIs [ON]" : "APIs [OFF]", TEVector2(80.0f, 24.0f)))
    {
        m_ShowAPIBrowser = !m_ShowAPIBrowser;
    }

    TimeGUI::Separator();

    // Check active hooks in the AST
    bool hasReady = false;
    bool hasUpdate = false;
    bool hasCollision = false;
    bool hasInput = false;

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
        TimeGUI::TextColored(TEColor(1.0f, 0.3f, 0.3f, 1.0f), "Status: Syntax Error at Line %d, Col %d",
                             scriptAsset->ErrorLine, scriptAsset->ErrorColumn);
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

    // Rich Actionable Diagnostics Banner
    if (!scriptAsset->ASTValid && !scriptAsset->CompileError.empty())
    {
        TimeGUI::Spacing();
        TimeGUI::TextColored(TEColor(1.0f, 0.35f, 0.35f, 1.0f), "[Error Details]: %s",
                             scriptAsset->CompileError.c_str());
        if (!scriptAsset->CompileSuggestion.empty())
        {
            TimeGUI::TextColored(TEColor(0.3f, 0.9f, 0.7f, 1.0f), "💡 Suggested Fix: %s",
                                 scriptAsset->CompileSuggestion.c_str());
        }
    }

    TimeGUI::Separator();

    float totalAvailWidth = TimeGUI::GetContentRegionAvail().x;
    float editorAreaWidth = m_ShowAPIBrowser ? (totalAvailWidth - 280.0f) : totalAvailWidth;

    // Main UICodeEdit Widget
    TimeGUI::BeginChild("##CodeEditContainer", TEVector2(editorAreaWidth, -1.0f), false);
    {
        m_CodeEdit.SetSize(TEVector2(-1.0f, -1.0f));
        m_CodeEdit.Draw();
    }
    TimeGUI::EndChild();

    // Right Sidebar: API & Auto-Recommendations Browser
    if (m_ShowAPIBrowser)
    {
        TimeGUI::SameLine();
        TimeGUI::BeginChild("##APIBrowserPane", TEVector2(270.0f, -1.0f), true);
        {
            TimeGUI::TextColored(TEColor(0.3f, 0.9f, 0.6f, 1.0f), "✨ TScript API & Snippets");
            TimeGUI::TextDisabled("Click '+ Insert' to add to script");
            TimeGUI::Separator();

            TimeGUI::InputText("Search##APISearch", m_FilterBuffer);
            TimeGUI::Spacing();

            auto completions = TScriptAutoComplete::GetCompletions(m_FilterBuffer, "");
            for (size_t i = 0; i < completions.size(); i++)
            {
                const auto &item = completions[i];
                TEString btnId = "+##" + TEString::FromInt((int)i);

                if (TimeGUI::SmallButton(btnId.c_str()))
                {
                    m_CodeEdit.InsertTextAtCursor(item.InsertText);
                    scriptAsset->SourceText = m_CodeEdit.GetText();
                    AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
                    scriptAsset->Recompile();
                }
                TimeGUI::SameLine();

                TEColor kindColor(0.8f, 0.8f, 0.8f, 1.0f);
                if (item.Kind == TScriptCompletionKind::Hook)
                    kindColor = TEColor(0.9f, 0.9f, 0.4f, 1.0f);
                else if (item.Kind == TScriptCompletionKind::Function)
                    kindColor = TEColor(0.4f, 0.8f, 1.0f, 1.0f);
                else if (item.Kind == TScriptCompletionKind::Property)
                    kindColor = TEColor(0.7f, 0.5f, 1.0f, 1.0f);
                else if (item.Kind == TScriptCompletionKind::Snippet)
                    kindColor = TEColor(1.0f, 0.6f, 0.8f, 1.0f);

                TimeGUI::TextColored(kindColor, "%s", item.Label.c_str());
                if (!item.Detail.empty())
                {
                    TimeGUI::TextDisabled("  %s", item.Detail.c_str());
                }
                TimeGUI::Spacing();
            }
        }
        TimeGUI::EndChild();
    }
}

void TScriptAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
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

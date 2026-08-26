#include "DialogueGraphAssetEditor.hpp"
#include "Utils/TimeGUI.hpp"

void DialogueGraphAssetEditor::DrawToolbar(DialogueTreeAsset &asset, const TEString &assetPath)
{
    TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ChildBg, TEColor(0.15f, 0.15f, 0.18f, 1.0f));
    TimeGUI::BeginChild("##DlgToolbar", TEVector2(0.0f, 36.0f), false);

    // Save
    bool dirty = m_Canvas.IsDirty();
    if (dirty)
    {
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.7f, 0.35f, 0.0f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ButtonHovered, TEColor(0.9f, 0.5f, 0.0f, 1.0f));
    }
    if (TimeGUI::Button(dirty ? "  💾 Save*  " : "  💾 Save  "))
    {
        Save(asset, assetPath);
    }
    if (dirty)
        TimeGUI::PopStyleColor(2);

    TimeGUI::SameLine();
    TimeGUI::Separator();
    TimeGUI::SameLine();

    // Simulator toggle
    if (m_bShowSimulator)
    {
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.15f, 0.6f, 0.2f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ButtonHovered, TEColor(0.2f, 0.8f, 0.3f, 1.0f));
    }
    if (TimeGUI::Button(m_bShowSimulator ? "  ▶ Simulator [ON]  " : "  ▶ Simulator  "))
    {
        m_bShowSimulator = !m_bShowSimulator;
        if (!m_bShowSimulator)
            m_Simulator.StopSimulation();
    }
    if (m_bShowSimulator)
        TimeGUI::PopStyleColor(2);

    TimeGUI::SameLine();

    // Add node shortcut buttons
    if (TimeGUI::Button("+ Dialogue"))
    {
        m_Canvas.PushUndoState(asset.GetGraph());
        asset.GetGraph().AddNode(NarrativeNodeType::Dialogue, TEVector2(200.0f, 200.0f));
        AssetEditorRegistry::MarkAssetDirty(assetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("+ Choice"))
    {
        m_Canvas.PushUndoState(asset.GetGraph());
        asset.GetGraph().AddNode(NarrativeNodeType::Choice, TEVector2(200.0f, 300.0f));
        AssetEditorRegistry::MarkAssetDirty(assetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("+ Condition"))
    {
        m_Canvas.PushUndoState(asset.GetGraph());
        asset.GetGraph().AddNode(NarrativeNodeType::Condition, TEVector2(400.0f, 200.0f));
        AssetEditorRegistry::MarkAssetDirty(assetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("+ Action"))
    {
        m_Canvas.PushUndoState(asset.GetGraph());
        asset.GetGraph().AddNode(NarrativeNodeType::Action, TEVector2(400.0f, 300.0f));
        AssetEditorRegistry::MarkAssetDirty(assetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("+ Exit"))
    {
        m_Canvas.PushUndoState(asset.GetGraph());
        asset.GetGraph().AddNode(NarrativeNodeType::Exit, TEVector2(600.0f, 200.0f));
        AssetEditorRegistry::MarkAssetDirty(assetPath, true);
    }

    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();
}

void DialogueGraphAssetEditor::Save(DialogueTreeAsset &asset, const TEString &assetPath)
{
    if (asset.SaveToFile(assetPath))
    {
        m_Canvas.ClearDirty();
        AssetEditorRegistry::MarkAssetDirty(assetPath, false);
    }
}

void DialogueGraphAssetEditor::DrawEditor(EditorTab &tab)
{
    auto dialogueAsset = std::dynamic_pointer_cast<DialogueTreeAsset>(tab.LoadedAsset);
    if (!dialogueAsset)
    {
        dialogueAsset = CreateRef<DialogueTreeAsset>();
        dialogueAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = dialogueAsset;
    }
    if (!dialogueAsset)
        return;

    DialogueTreeAsset &asset = *dialogueAsset;

    DrawToolbar(asset, tab.AssetPath);

    // Main layout: left = canvas, right = inspector
    float inspectorWidth = 280.0f;
    float simPanelHeight = m_bShowSimulator ? 300.0f : 0.0f;
    float canvasWidth = TimeGUI::GetContentRegionAvail().x - inspectorWidth - 6.0f;
    float totalHeight = TimeGUI::GetContentRegionAvail().y;
    float graphHeight = totalHeight - simPanelHeight;

    // ── Graph Canvas ──────────────────────────────────────────────────────────
    uint64_t highlightID = m_bShowSimulator ? m_Simulator.GetActiveNodeID() : 0;

    TimeGUI::BeginChild("##DlgGraphCanvas", TEVector2(canvasWidth, graphHeight), false);
    m_Canvas.Draw(asset.GetGraph(), highlightID);
    TimeGUI::EndChild();

    TimeGUI::SameLine();

    // ── Node Inspector ────────────────────────────────────────────────────────
    TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ChildBg, TEColor(0.12f, 0.12f, 0.15f, 1.0f));
    TimeGUI::BeginChild("##DlgInspector", TEVector2(inspectorWidth, graphHeight), true);
    TimeGUI::Text("  Inspector");
    TimeGUI::Separator();
    m_Inspector.DrawInspector(asset.GetGraph(), m_Canvas.GetSelectedNodeID());
    TimeGUI::EndChild();
    TimeGUI::PopStyleColor();

    // ── Live Simulator Panel ──────────────────────────────────────────────────
    if (m_bShowSimulator)
    {
        TimeGUI::Separator();
        TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ChildBg, TEColor(0.10f, 0.14f, 0.10f, 1.0f));
        TimeGUI::BeginChild("##DlgSimPanel", TEVector2(0.0f, simPanelHeight), true);
        TimeGUI::Text("  ▶ Live Story Simulator");
        TimeGUI::Separator();
        m_Simulator.DrawSimulatorPanel(asset.GetGraph());
        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();
    }

    if (m_Canvas.IsDirty())
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
}

void DialogueGraphAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Coral / Rose
    dl.AddRectFilled(min, max, IM_COL32(230, 80, 110, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(36, 20, 26, 255), 2.0f);

    // Dialogue speech bubble relative to icon bounds
    float cx = min.x + w * 0.5f;
    float cy = min.y + h * 0.46f;
    float bw = w * 0.26f;
    float bh = h * 0.18f;
    dl.AddRectFilled(TEVector2(cx - bw, cy - bh), TEVector2(cx + bw, cy + bh), IM_COL32(255, 140, 170, 255), 3.0f);
    dl.AddTriangleFilled(TEVector2(cx - bw * 0.5f, cy + bh), TEVector2(cx, cy + bh),
                         TEVector2(cx - bw * 0.7f, cy + bh + h * 0.12f), IM_COL32(255, 140, 170, 255));
}

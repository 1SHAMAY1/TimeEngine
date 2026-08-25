#include "Editor/PCGGraphAssetEditor.hpp"
#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Nodes/PCGNodeRegistry.hpp"
#include "Utils/TimeGUI.hpp"

PCGGraphAssetEditor::PCGGraphAssetEditor()
{
}

void PCGGraphAssetEditor::DrawToolbar(PCGGraphAsset &asset, const TEString &assetPath)
{
    if (TimeGUI::Button("Save Graph"))
    {
        if (asset.SaveToFile(assetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(assetPath, false);
            TE_CORE_INFO("[PCGGraphAssetEditor] Saved PCG graph to: {}", assetPath);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Execute Test"))
    {
        PCGExecutionContext ctx(1337, glm::vec3(-50.0f, -50.0f, 0.0f), glm::vec3(50.0f, 50.0f, 0.0f));
        if (asset.GetGraph().Execute(ctx))
        {
            size_t totalPoints = 0;
            for (auto it = ctx.PinDataCache.begin(); it != ctx.PinDataCache.end(); ++it)
            {
                if (it->second)
                    totalPoints += it->second->GetCount();
            }
            m_LastGeneratedPointCount = totalPoints;
            TE_CORE_INFO("[PCGGraphAssetEditor] Test execution completed! Points generated in cache: {}", totalPoints);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Zoom to Fit"))
    {
        m_Canvas.ZoomToFit(asset.GetGraph());
    }

    TimeGUI::SameLine();
    TimeGUI::TextColored(TEColor(0.2f, 0.85f, 0.95f, 1.0f), " | Cached Points: %zu", m_LastGeneratedPointCount);
}

void PCGGraphAssetEditor::DrawNodeInspector(Graph &graph)
{
    auto selected = m_Canvas.GetPrimarySelectedNode(graph);
    if (!selected)
    {
        TimeGUI::TextDisabled("Select a node to inspect properties.");
        return;
    }

    TimeGUI::TextColored(TEColor(0.95f, 0.75f, 0.2f, 1.0f), "Node: %s (ID: %llu)", selected->Title.c_str(), selected->ID);
    TimeGUI::Text("Type: %s | Category: %s", selected->NodeType.c_str(), selected->Category.c_str());
    TimeGUI::Separator();

    for (auto it = selected->Properties.begin(); it != selected->Properties.end(); ++it)
    {
        TEString key = it->first;
        TEString val = it->second;

        if (TimeGUI::InputText(key.c_str(), val))
        {
            selected->SetProperty(key, val);
        }
    }
}

void PCGGraphAssetEditor::DrawEditor(EditorTab &tab)
{
    if (!m_PaletteRegistered)
    {
        PCGNodeRegistry::RegisterAllNodes(m_Canvas.GetPalette());
        m_PaletteRegistered = true;
    }

    TERef<PCGGraphAsset> asset;
    auto foundAsset = m_LoadedAssets.Find(tab.AssetPath);
    if (!foundAsset)
    {
        asset = CreateRef<PCGGraphAsset>();
        if (!tab.AssetPath.empty())
        {
            asset->LoadFromFile(tab.AssetPath);
        }
        m_LoadedAssets[tab.AssetPath] = asset;
    }
    else
    {
        asset = *foundAsset;
    }

    if (!asset)
        return;

    DrawToolbar(*asset, tab.AssetPath);
    TimeGUI::Separator();

    // Split into Canvas and Property Inspector
    float availWidth = TimeGUI::GetContentRegionAvail().x;
    float availHeight = TimeGUI::GetContentRegionAvail().y;

    float inspectorWidth = 260.0f;
    float canvasWidth = std::max(200.0f, availWidth - inspectorWidth - 10.0f);

    TimeGUI::BeginChild("##PCGCanvasView", TEVector2(canvasWidth, availHeight), true);
    m_Canvas.Draw(asset->GetGraph());
    TimeGUI::EndChild();

    TimeGUI::SameLine();

    TimeGUI::BeginChild("##PCGNodeInspector", TEVector2(inspectorWidth, availHeight), true);
    TimeGUI::Text("Node Properties");
    TimeGUI::Separator();
    DrawNodeInspector(asset->GetGraph());
    TimeGUI::EndChild();
}

void PCGGraphAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Cyan/Emerald
    dl.AddRectFilled(min, max, IM_COL32(0, 170, 180, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(16, 32, 36, 255), 2.0f);

    // Procedural terrain contour curve
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddLine(TEVector2(iMin.x + 4.0f, cy + 5.0f), TEVector2(cx - 3.0f, cy - 6.0f), IM_COL32(50, 240, 220, 255), 1.5f);
    dl.AddLine(TEVector2(cx - 3.0f, cy - 6.0f), TEVector2(cx + 4.0f, cy + 2.0f), IM_COL32(50, 240, 220, 255), 1.5f);
    dl.AddLine(TEVector2(cx + 4.0f, cy + 2.0f), TEVector2(iMax.x - 4.0f, cy - 4.0f), IM_COL32(50, 240, 220, 255), 1.5f);
    dl.AddCircleFilled(TEVector2(cx - 3.0f, cy - 6.0f), 3.0f, IM_COL32(255, 220, 80, 255));
    dl.AddCircleFilled(TEVector2(iMax.x - 4.0f, cy - 4.0f), 3.0f, IM_COL32(255, 220, 80, 255));
}

#include "Editor/StateTreeAssetEditor.hpp"
#include "Core/AI/StateTreeGraph.hpp"
#include "Core/Asset/StateTreeAsset.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/TimeGUI.hpp"

StateTreeAssetEditor::StateTreeAssetEditor() {}

void StateTreeAssetEditor::DrawEditor(EditorTab &tab)
{
    auto asset = std::dynamic_pointer_cast<StateTreeAsset>(tab.LoadedAsset);
    if (!asset)
    {
        asset = CreateRef<StateTreeAsset>();
        asset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = asset;
    }
    if (!asset || !asset->GetGraph())
        return;

    auto graph = asset->GetGraph();

    // Top Toolbar
    if (TimeGUI::Button("Save StateTree"))
    {
        if (asset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Add State"))
    {
        graph->CreateState("NewState", {200.0f, 150.0f});
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Add Root"))
    {
        graph->CreateRootNode({50.0f, 150.0f});
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Zoom To Fit"))
    {
        m_Canvas.ZoomToFit(*graph);
    }

    TimeGUI::Separator();

    TimeGUI::Columns(2, "StateTreeEditorSplitter", true);

    // Left Column: Node Canvas
    TimeGUI::BeginChild("StateTreeCanvasChild", TEVector2(0.0f, 0.0f), false);
    m_Canvas.Draw(*graph);
    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // Right Column: State Node Inspector
    TimeGUI::BeginChild("StateTreeInspectorChild", TEVector2(0.0f, 0.0f), false);
    TimeGUI::Text("State Inspector");
    TimeGUI::Separator();

    auto selectedNode = m_Canvas.GetPrimarySelectedNode(*graph);
    if (selectedNode)
    {
        m_SelectedNodeID = selectedNode->ID;
        auto stateNode = std::dynamic_pointer_cast<StateNode>(selectedNode);
        if (stateNode)
        {
            TimeGUI::Text("Node Type: StateNode");
            TimeGUI::Text("ID: %llu", (unsigned long long)stateNode->ID);

            TEString stateName = stateNode->StateName;
            if (TimeGUI::InputText("State Name", stateName))
            {
                stateNode->StateName = stateName;
                stateNode->Title = stateName;
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            }

            TimeGUI::Separator();
            TimeGUI::Text("Enter Tasks (%d)", (int)stateNode->EnterTasks.size());
            for (size_t i = 0; i < stateNode->EnterTasks.size(); ++i)
            {
                if (stateNode->EnterTasks[i])
                {
                    TimeGUI::Text("  • %s", stateNode->EnterTasks[i]->GetTaskType().c_str());
                }
            }

            TimeGUI::Separator();
            TimeGUI::Text("Tick Tasks (%d)", (int)stateNode->TickTasks.size());
            for (size_t i = 0; i < stateNode->TickTasks.size(); ++i)
            {
                if (stateNode->TickTasks[i])
                {
                    TimeGUI::Text("  • %s", stateNode->TickTasks[i]->GetTaskType().c_str());
                }
            }

            TimeGUI::Separator();
            TimeGUI::Text("Exit Tasks (%d)", (int)stateNode->ExitTasks.size());
            for (size_t i = 0; i < stateNode->ExitTasks.size(); ++i)
            {
                if (stateNode->ExitTasks[i])
                {
                    TimeGUI::Text("  • %s", stateNode->ExitTasks[i]->GetTaskType().c_str());
                }
            }

            TimeGUI::Separator();
            TimeGUI::Text("Outgoing Transitions");
            if (!stateNode->OutputPins.empty())
            {
                auto conns = graph->GetConnectionsForPin(stateNode->OutputPins[0].ID);
                for (size_t i = 0; i < conns.size(); ++i)
                {
                    auto target = graph->FindNode(conns[i].TargetNodeID);
                    TEString targetName = target ? target->Title : "Unknown";
                    TimeGUI::Text("  -> %s (Pin %llu)", targetName.c_str(), (unsigned long long)conns[i].TargetPinID);
                }
            }
        }
        else
        {
            TimeGUI::Text("Node Type: %s", selectedNode->NodeType.c_str());
            TimeGUI::Text("Title: %s", selectedNode->Title.c_str());
        }
    }
    else
    {
        TimeGUI::TextDisabled("Select a State Node to edit tasks and transitions.");
    }

    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

void StateTreeAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Purple/Plum
    dl.AddRectFilled(min, max, IM_COL32(130, 50, 180, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(26, 18, 36, 255), 2.0f);

    // State Tree Graph Nodes & Link
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddLine(TEVector2(iMin.x + 6.0f, cy), TEVector2(iMax.x - 6.0f, cy), IM_COL32(180, 120, 255, 200), 1.5f);
    dl.AddCircleFilled(TEVector2(iMin.x + 6.0f, cy), 4.0f, IM_COL32(255, 160, 50, 255));
    dl.AddCircleFilled(TEVector2(cx, cy), 4.0f, IM_COL32(140, 220, 255, 255));
    dl.AddCircleFilled(TEVector2(iMax.x - 6.0f, cy), 4.0f, IM_COL32(80, 240, 140, 255));
}

TE_REGISTER_ASSET_EDITOR(StateTreeAssetEditor);

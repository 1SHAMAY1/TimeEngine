#include "Editor/MaterialAssetEditor.hpp"
#include "Asset/MaterialAsset.hpp"
#include "Asset/MaterialAssetSerializer.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Graph/MaterialNodeRegistry.hpp"
#include "Utils/TimeGUI.hpp"


MaterialAssetEditor::MaterialAssetEditor()
{
}

void MaterialAssetEditor::DrawDetailsPanel(Graph &graph, const TERef<GraphNode> &selectedNode)
{
    TimeGUI::Text("Node Details");
    TimeGUI::Separator();

    if (!selectedNode)
    {
        TimeGUI::TextDisabled("Select a node to inspect properties.");
        return;
    }

    TimeGUI::Text("Type: %s", selectedNode->NodeType.c_str());
    TimeGUI::Text("Title: %s", selectedNode->Title.c_str());
    TimeGUI::Separator();

    // Node Properties
    for (auto &prop : selectedNode->Properties)
    {
        TEString val = prop.second;
        if (TimeGUI::InputText(prop.first.c_str(), val))
        {
            selectedNode->SetProperty(prop.first, val);
        }
    }
}

void MaterialAssetEditor::DrawEditor(EditorTab &tab)
{
    auto material = std::dynamic_pointer_cast<MaterialAsset>(tab.LoadedAsset);
    if (!material)
    {
        material = CreateRef<MaterialAsset>();
        MaterialAssetSerializer::DeserializeMaterial(*material, tab.AssetPath);
        tab.LoadedAsset = material;
    }
    if (!material)
        return;

    if (!m_PaletteInitialized)
    {
        MaterialNodeRegistry::RegisterAllNodes(m_Canvas.GetPalette());
        m_PaletteInitialized = true;
    }

    // Top Toolbar
    if (TimeGUI::Button("Compile Shader"))
    {
        material->Recompile();
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset"))
    {
        if (MaterialAssetSerializer::SerializeMaterial(*material, tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Zoom to Fit"))
    {
        m_Canvas.ZoomToFit(material->GetGraph());
    }

    TimeGUI::Separator();

    // Splitter: Left (Viewport + Details), Right (Node Canvas)
    TimeGUI::Columns(2, "MaterialEditorSplitter", true);
    TimeGUI::SetColumnWidth(0, 320.0f);

    // Left Column
    TimeGUI::BeginChild("MaterialLeftColumn", TEVector2(0, 0), false);
    m_PreviewViewport.Draw(material);
    TimeGUI::Separator();
    DrawDetailsPanel(material->GetGraph(), m_Canvas.GetPrimarySelectedNode(material->GetGraph()));
    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // Right Column: Node Canvas
    TimeGUI::BeginChild("MaterialRightColumn", TEVector2(0, 0), false);
    m_Canvas.Draw(material->GetGraph());
    TimeGUI::EndChild();

    TimeGUI::Columns(1);
}

void MaterialAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;

    // Card background in Emerald/Teal
    dl.AddRectFilled(min, max, IM_COL32(30, 180, 140, 230), 4.0f);
    // 3D Shader Sphere
    float cx = (min.x + max.x) * 0.5f;
    float cy = (min.y + max.y) * 0.5f;
    float r = w * 0.32f;
    dl.AddCircleFilled(TEVector2(cx, cy), r, IM_COL32(20, 90, 80, 255));
    dl.AddCircleFilled(TEVector2(cx - r * 0.2f, cy - r * 0.2f), r * 0.65f, IM_COL32(40, 220, 170, 255));
    dl.AddCircleFilled(TEVector2(cx - r * 0.35f, cy - r * 0.35f), r * 0.25f, IM_COL32(230, 255, 245, 255));
}

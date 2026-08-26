#include "Editor/Panels/UIAssetEditor.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

UIAssetEditor::UIAssetEditor() {}

TEString UIAssetEditor::CreateDefaultTemplate(const TEString &name) const
{
    return "{\n"
           "  \"Name\": \"" +
           name +
           "\",\n"
           "  \"AssetType\": \"UIAsset\",\n"
           "  \"Description\": \"TimeEngine UI Layout Asset\",\n"
           "  \"Nodes\": [\n"
           "    {\n"
           "      \"Type\": \"UIBorder\",\n"
           "      \"ID\": \"RootBorder\",\n"
           "      \"Position\": [0.0, 0.0],\n"
           "      \"Size\": [400.0, 300.0],\n"
           "      \"Text\": \"\",\n"
           "      \"Color\": [0.15, 0.18, 0.25, 0.95],\n"
           "      \"ParentIndex\": -1\n"
           "    },\n"
           "    {\n"
           "      \"Type\": \"UIButton\",\n"
           "      \"ID\": \"ActionBtn\",\n"
           "      \"Position\": [20.0, 20.0],\n"
           "      \"Size\": [120.0, 35.0],\n"
           "      \"Text\": \"Click Me\",\n"
           "      \"Color\": [0.25, 0.55, 0.90, 1.0],\n"
           "      \"ParentIndex\": 0\n"
           "    }\n"
           "  ]\n"
           "}";
}

void UIAssetEditor::DrawEditor(EditorTab &tab)
{
    auto uiAsset = std::dynamic_pointer_cast<UIAsset>(tab.LoadedAsset);
    if (!uiAsset)
    {
        uiAsset = CreateRef<UIAsset>();
        uiAsset->LoadFromFile(tab.AssetPath);
        tab.LoadedAsset = uiAsset;
        m_PreviewRoot = uiAsset->InstantiateWidgetTree();
        m_SelectedNodeIndex = -1;
        m_IsDirty = false;
    }

    if (!m_PreviewRoot && uiAsset)
    {
        m_PreviewRoot = uiAsset->InstantiateWidgetTree();
    }

    // Top Toolbar
    TimeGUI::TextColored(TEVector4(0.3f, 0.8f, 0.4f, 1.0f), "UI Asset Designer: %s", tab.AssetPath.c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Asset##UISave"))
    {
        if (uiAsset && !tab.AssetPath.empty())
        {
            if (uiAsset->SaveToFile(tab.AssetPath))
            {
                m_PreviewRoot = uiAsset->InstantiateWidgetTree();
                m_IsDirty = false;
                AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
            }
        }
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Reload##UIReload"))
    {
        if (uiAsset)
        {
            uiAsset->LoadFromFile(tab.AssetPath);
            m_PreviewRoot = uiAsset->InstantiateWidgetTree();
            m_SelectedNodeIndex = -1;
            m_IsDirty = false;
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }
    TimeGUI::Separator();

    // 3-Column Layout: Palette & Hierarchy | Canvas Preview | Inspector
    TimeGUI::Columns(3, "UIEditorColumns", true);

    // Column 1: Palette & Hierarchy
    RenderPalette(*uiAsset, m_PreviewRoot, m_IsDirty, m_SelectedNodeIndex);
    TimeGUI::Spacing();
    RenderHierarchy(*uiAsset, m_SelectedNodeIndex);

    TimeGUI::NextColumn();

    // Column 2: Canvas Preview
    RenderCanvasPreview(m_PreviewRoot);

    TimeGUI::NextColumn();

    // Column 3: Inspector
    RenderInspector(*uiAsset, m_PreviewRoot, m_IsDirty, m_SelectedNodeIndex);

    if (m_IsDirty)
    {
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Columns(1);
}

void UIAssetEditor::RenderPalette(UIAsset &asset, TERef<UIWidget> &previewRoot, bool &isDirty, int selectedNodeIndex)
{
    TimeGUI::TextColored(TEVector4(0.4f, 0.7f, 0.95f, 1.0f), "Widget Palette");
    TimeGUI::Separator();

    auto AddWidgetNode = [&](const TEString &type, const TEString &name, const TEVector2 &size)
    {
        UIWidgetNodeDef node;
        node.Type = type;
        node.ID = name + "_" + TEString::FromInt(static_cast<int>(asset.GetNodes().Num() + 1));
        node.Size = size;
        node.ParentIndex = selectedNodeIndex;
        asset.AddNode(node);
        previewRoot = asset.InstantiateWidgetTree();
        isDirty = true;
    };

    if (TimeGUI::Button("+ Button##PaletteBtn", TEVector2(120, 25)))
    {
        AddWidgetNode("UIButton", "Button", {120, 35});
    }
    if (TimeGUI::Button("+ Slider##PaletteSlider", TEVector2(120, 25)))
    {
        AddWidgetNode("UISlider", "Slider", {150, 25});
    }
    if (TimeGUI::Button("+ Border##PaletteBorder", TEVector2(120, 25)))
    {
        AddWidgetNode("UIBorder", "Border", {200, 150});
    }
    if (TimeGUI::Button("+ ScrollBox##PaletteScroll", TEVector2(120, 25)))
    {
        AddWidgetNode("UIScrollBox", "ScrollBox", {200, 150});
    }
    if (TimeGUI::Button("+ SizeBox##PaletteSize", TEVector2(120, 25)))
    {
        AddWidgetNode("UISizeBox", "SizeBox", {100, 50});
    }
}

void UIAssetEditor::RenderHierarchy(UIAsset &asset, int &selectedNodeIndex)
{
    TimeGUI::TextColored(TEVector4(0.4f, 0.7f, 0.95f, 1.0f), "Widget Tree");
    TimeGUI::Separator();

    const auto &nodes = asset.GetNodes();
    for (size_t i = 0; i < nodes.Num(); ++i)
    {
        const auto &node = nodes[i];
        TEString label = "[" + node.Type + "] " + node.ID;
        bool isSelected = (selectedNodeIndex == static_cast<int>(i));

        if (TimeGUI::Selectable(label.c_str(), isSelected))
        {
            selectedNodeIndex = static_cast<int>(i);
        }
    }
}

void UIAssetEditor::RenderCanvasPreview(TERef<UIWidget> previewRoot)
{
    TimeGUI::TextColored(TEVector4(0.4f, 0.7f, 0.95f, 1.0f), "Live Canvas Preview");
    TimeGUI::Separator();

    TimeGUI::BeginChild("UICanvasPreviewRegion", TEVector2(0, 0), true);
    if (previewRoot)
    {
        previewRoot->Draw();
    }
    else
    {
        TimeGUI::TextDisabled("No widgets in UI asset. Add widgets from the palette.");
    }
    TimeGUI::EndChild();
}

void UIAssetEditor::RenderInspector(UIAsset &asset, TERef<UIWidget> &previewRoot, bool &isDirty, int selectedNodeIndex)
{
    TimeGUI::TextColored(TEVector4(0.4f, 0.7f, 0.95f, 1.0f), "Widget Details");
    TimeGUI::Separator();

    if (selectedNodeIndex >= 0 && selectedNodeIndex < static_cast<int>(asset.GetNodes().Num()))
    {
        // Mutate selected node
        auto &nodes = const_cast<TEArray<UIWidgetNodeDef> &>(asset.GetNodes());
        auto &node = nodes[selectedNodeIndex];

        TimeGUI::Text("Type: %s", node.Type.c_str());

        if (TimeGUI::InputText("ID##NodeID", node.ID))
        {
            isDirty = true;
        }

        if (node.Type == "UIButton" || node.Type == "UISlider")
        {
            if (TimeGUI::InputText("Label / Text##NodeText", node.Text))
            {
                isDirty = true;
            }
        }

        float size[2] = {node.Size.x, node.Size.y};
        if (TimeGUI::DragFloat2("Size", size, 1.0f, 0.0f, 2000.0f))
        {
            node.Size = {size[0], size[1]};
            isDirty = true;
        }

        float pos[2] = {node.Position.x, node.Position.y};
        if (TimeGUI::DragFloat2("Position", pos, 1.0f, -2000.0f, 2000.0f))
        {
            node.Position = {pos[0], pos[1]};
            isDirty = true;
        }

        if (node.Type == "UISlider")
        {
            TimeGUI::DragFloat("Min Value", &node.MinValue, 0.1f);
            TimeGUI::DragFloat("Max Value", &node.MaxValue, 0.1f);
            TimeGUI::SliderFloat("Value", &node.Value, node.MinValue, node.MaxValue);
        }

        if (isDirty)
        {
            previewRoot = asset.InstantiateWidgetTree();
        }
    }
    else
    {
        TimeGUI::TextDisabled("Select a widget node to view properties.");
    }
}

void UIAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.10f;

    // Canvas background in Indigo
    dl.AddRectFilled(min, max, IM_COL32(50, 75, 140, 230), 4.0f);

    // Inner Canvas frame
    TEVector2 cMin(min.x + pad, min.y + pad);
    TEVector2 cMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(cMin, cMax, IM_COL32(20, 25, 40, 255), 2.0f);

    // Top UI Header bar
    float headerH = (cMax.y - cMin.y) * 0.22f;
    dl.AddRectFilled(cMin, TEVector2(cMax.x, cMin.y + headerH), IM_COL32(60, 110, 210, 255), 2.0f);

    // UI Buttons
    float btnY = cMin.y + headerH + 4.0f;
    float btnH = (cMax.y - btnY - 4.0f) * 0.40f;
    dl.AddRectFilled(TEVector2(cMin.x + 4.0f, btnY), TEVector2(cMax.x - 4.0f, btnY + btnH), IM_COL32(70, 140, 90, 255),
                     2.0f);
    dl.AddRectFilled(TEVector2(cMin.x + 4.0f, btnY + btnH + 3.0f), TEVector2(cMax.x - 4.0f, btnY + btnH * 2.0f + 3.0f),
                     IM_COL32(180, 100, 40, 255), 2.0f);
}

TE_REGISTER_ASSET_EDITOR(UIAssetEditor);

#include "Core/PreRequisites.h"
#include "Core/Asset/UIAsset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

UIAsset::UIAsset()
{
}

UIAsset::UIAsset(const TEString &name)
    : m_Name(name)
{
}

TERef<Asset> UIAsset::Clone() const
{
    auto clone = CreateRef<UIAsset>(m_Name);
    clone->m_Handle = m_Handle;
    clone->m_Nodes = m_Nodes;
    return clone;
}

bool UIAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
        return false;

    m_Handle = AssetRegistry::RegisterPath(path);
    m_Nodes.Clear();

    return TEFileSystem::ForEachLine(path, [this](const TEString &line) {
        TEString trimmed = line.Trim();
        if (trimmed.empty() || trimmed.StartsWith("#"))
            return true;

        if (trimmed.StartsWith("Node: "))
        {
            TEString content = trimmed.Mid(6);
            auto parts = content.Split(" ");
            if (parts.Num() >= 14)
            {
                UIWidgetNodeDef node;
                node.Type = parts[0];
                node.ID = parts[1];
                node.Position.x = parts[2].ToFloat();
                node.Position.y = parts[3].ToFloat();
                node.Size.x = parts[4].ToFloat();
                node.Size.y = parts[5].ToFloat();
                node.Color.x = parts[6].ToFloat();
                node.Color.y = parts[7].ToFloat();
                node.Color.z = parts[8].ToFloat();
                node.Color.w = parts[9].ToFloat();
                node.Value = parts[10].ToFloat();
                node.MinValue = parts[11].ToFloat();
                node.MaxValue = parts[12].ToFloat();
                node.ParentIndex = parts[13].ToInt();

                if (parts.Num() > 14)
                {
                    TEString textCombined;
                    for (size_t p = 14; p < parts.Num(); ++p)
                    {
                        if (p > 14)
                            textCombined += " ";
                        textCombined += parts[p];
                    }
                    node.Text = textCombined;
                }
                m_Nodes.Add(node);
            }
        }
        return true;
    });
}

bool UIAsset::SaveToFile(const TEString &path)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
        return false;

    file << "# TimeEngine UI Layout Asset\n";
    for (size_t i = 0; i < m_Nodes.Num(); ++i)
    {
        const auto &n = m_Nodes[i];
        file << "Node: " << n.Type.c_str() << " " << n.ID.c_str() << " " << n.Position.x << " " << n.Position.y << " "
             << n.Size.x << " " << n.Size.y << " " << n.Color.x << " " << n.Color.y << " " << n.Color.z << " " << n.Color.w << " "
             << n.Value << " " << n.MinValue << " " << n.MaxValue << " " << n.ParentIndex << " " << n.Text.c_str() << "\n";
    }
    return true;
}

void UIAsset::OnContentBrowserCreate(const TEString &path)
{
    TEString baseName = "NewUI";
    TEString finalPath = path / (baseName + ".teui");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".teui");
    }

    // Default template: Border root with a button
    UIWidgetNodeDef rootBorder;
    rootBorder.Type = "UIBorder";
    rootBorder.ID = "Root";
    rootBorder.Size = {300.0f, 200.0f};
    rootBorder.ParentIndex = -1;
    m_Nodes.Add(rootBorder);

    UIWidgetNodeDef btn;
    btn.Type = "UIButton";
    btn.ID = "SubmitBtn";
    btn.Text = "Click Me";
    btn.Size = {120.0f, 35.0f};
    btn.ParentIndex = 0;
    m_Nodes.Add(btn);

    SaveToFile(finalPath);
}

TERef<UIWidget> UIAsset::InstantiateWidgetTree() const
{
    if (m_Nodes.Num() == 0)
        return nullptr;

    TEArray<TERef<UIWidget>> createdWidgets;
    createdWidgets.Reserve(m_Nodes.Num());

    for (size_t i = 0; i < m_Nodes.Num(); ++i)
    {
        const auto &def = m_Nodes[i];
        TERef<UIWidget> widget = nullptr;

        if (def.Type == "UIButton")
        {
            widget = CreateRef<UIButton>(def.Text.empty() ? def.ID : def.Text, def.Size, def.ID);
        }
        else if (def.Type == "UISlider")
        {
            widget = CreateRef<UISlider>(def.Text.empty() ? def.ID : def.Text, def.MinValue, def.MaxValue, def.ID);
        }
        else if (def.Type == "UIScrollBox")
        {
            widget = CreateRef<UIScrollBox>(def.Size, def.ID);
        }
        else if (def.Type == "UISizeBox")
        {
            widget = CreateRef<UISizeBox>(def.Size, def.ID);
        }
        else
        {
            // Default UIBorder container
            widget = CreateRef<UIBorder>(def.Color, TEVector4(0.2f, 0.2f, 0.2f, 1.0f), 6.0f, def.ID);
            widget->SetSize(def.Size);
        }

        if (widget)
        {
            widget->SetPosition(def.Position);
        }
        createdWidgets.Add(widget);
    }

    // Build parent-child relationships
    TERef<UIWidget> rootWidget = nullptr;
    for (size_t i = 0; i < m_Nodes.Num(); ++i)
    {
        int pIndex = m_Nodes[i].ParentIndex;
        if (pIndex >= 0 && pIndex < static_cast<int>(createdWidgets.Num()))
        {
            if (createdWidgets[pIndex] && createdWidgets[i])
            {
                createdWidgets[pIndex]->AddChild(createdWidgets[i]);
            }
        }
        else if (!rootWidget && createdWidgets[i])
        {
            rootWidget = createdWidgets[i];
        }
    }

    return rootWidget ? rootWidget : (createdWidgets.Num() > 0 ? createdWidgets[0] : nullptr);
}


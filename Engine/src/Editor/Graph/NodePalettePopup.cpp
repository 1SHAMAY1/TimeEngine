#include "Core/PreRequisites.h"
#include "Editor/Graph/NodePalettePopup.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cstring>


void NodePalettePopup::Clear()
{
    m_Entries.Empty();
}

void NodePalettePopup::RegisterNode(const NodePaletteEntry &entry)
{
    m_Entries.Add(entry);
}

void NodePalettePopup::RegisterNode(const TEString &name, const TEString &category, const TEString &nodeType,
                                    const TEString &desc, std::function<TERef<GraphNode>()> factory)
{
    NodePaletteEntry entry;
    entry.Name = name;
    entry.Category = category;
    entry.NodeType = nodeType;
    entry.Description = desc;
    entry.Factory = factory;
    m_Entries.Add(entry);
}

void NodePalettePopup::Open(const TEVector2 &spawnGraphPos)
{
    m_IsOpen = true;
    m_JustOpened = true;
    m_SpawnGraphPos = spawnGraphPos;
    m_SearchBuffer = "";
}

TERef<GraphNode> NodePalettePopup::Draw()
{
    if (!m_IsOpen)
        return nullptr;

    TERef<GraphNode> spawnedNode = nullptr;

    if (m_JustOpened)
    {
        TimeGUI::OpenPopup("NodePaletteSearchPopup");
        m_JustOpened = false;
    }

    TimeGUI::SetNextWindowSize(TEVector2(320.0f, 380.0f), TimeGUI::TimeGUICond_Appearing);
    if (TimeGUI::BeginPopup("NodePaletteSearchPopup"))
    {
        TimeGUI::Text("Add Node");
        TimeGUI::Separator();

        TimeGUI::SetNextItemWidth(-1.0f);
        TimeGUI::InputTextWithHint("##NodeSearch", "Search nodes...", m_SearchBuffer, sizeof(m_SearchBuffer));

        TEString searchLower = m_SearchBuffer.ToLower();
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        TimeGUI::Separator();
        TimeGUI::BeginChild("NodePaletteList", TEVector2(0, 0), false);

        // Group entries by category
        TEMap<TEString, TEArray<NodePaletteEntry>> categories;
        for (const auto &entry : m_Entries)
        {
            if (!searchLower.empty())
            {
                TEString nameLower = entry.Name.ToLower();
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                TEString catLower = entry.Category.ToLower();
                std::transform(catLower.begin(), catLower.end(), catLower.begin(), ::tolower);

                if (!nameLower.Contains(searchLower) &&
                    !catLower.Contains(searchLower))
                {
                    continue;
                }
            }

            categories[entry.Category].Add(entry);
        }

        for (const auto &pair : categories)
        {
            bool showCat = searchLower.empty() ? TimeGUI::TreeNodeEx(pair.first) : true;
            if (showCat)
            {
                for (const auto &entry : pair.second)
                {
                    if (TimeGUI::Selectable(entry.Name))
                    {
                        if (entry.Factory)
                        {
                            spawnedNode = entry.Factory();
                            if (spawnedNode)
                            {
                                spawnedNode->Position = m_SpawnGraphPos;
                            }
                        }
                        m_IsOpen = false;
                        TimeGUI::CloseCurrentPopup();
                        break;
                    }
                    if (!entry.Description.empty() && TimeGUI::IsItemHovered())
                    {
                        TimeGUI::SetTooltip(entry.Description.c_str());
                    }
                }
                if (searchLower.empty())
                    TimeGUI::TreePop();
            }
            if (spawnedNode)
                break;
        }

        TimeGUI::EndChild();
        TimeGUI::EndPopup();
    }
    else
    {
        m_IsOpen = false;
    }

    return spawnedNode;
}

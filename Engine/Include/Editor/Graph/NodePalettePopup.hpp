#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <functional>


struct TE_API NodePaletteEntry
{
    TEString Name;
    TEString Category;
    TEString NodeType;
    TEString Description;
    std::function<TERef<GraphNode>()> Factory;
};

class TE_API NodePalettePopup
{
public:
    NodePalettePopup() = default;

    void Clear();
    void RegisterNode(const NodePaletteEntry &entry);
    void RegisterNode(const TEString &name, const TEString &category, const TEString &nodeType,
                      const TEString &desc, std::function<TERef<GraphNode>()> factory);

    void Open(const TEVector2 &spawnGraphPos);
    bool IsOpen() const { return m_IsOpen; }
    void Close() { m_IsOpen = false; }

    // Returns created node if user selected one this frame
    TERef<GraphNode> Draw();

private:
    bool m_IsOpen = false;
    bool m_JustOpened = false;
    TEVector2 m_SpawnGraphPos = {0.0f, 0.0f};
    TEString m_SearchBuffer = "";
    TEArray<NodePaletteEntry> m_Entries;
};

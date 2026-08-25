#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/Graph.hpp"
#include "Editor/Graph/NodeCanvasStyle.hpp"
#include "Editor/Graph/NodePalettePopup.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include <functional>


using NodeBodyDrawerFn = std::function<void(const TERef<GraphNode> &node)>;

class TE_API NodeCanvas
{
public:
    NodeCanvas();
    virtual ~NodeCanvas() = default;

    void SetStyle(const NodeCanvasStyle &style) { m_Style = style; }
    const NodeCanvasStyle &GetStyle() const { return m_Style; }

    NodePalettePopup &GetPalette() { return m_Palette; }

    void SetCustomNodeBodyDrawer(NodeBodyDrawerFn drawer) { m_CustomBodyDrawer = drawer; }

    // Renders the interactive graph canvas inside current TimeGUI window/child
    void Draw(Graph &graph, const TEVector2 &canvasSize = {0.0f, 0.0f});

    // Selection
    const TESet<uint64_t> &GetSelectedNodes() const { return m_SelectedNodeIDs; }
    void SelectNode(uint64_t nodeId, bool clearOthers = true);
    void ClearSelection();
    TERef<GraphNode> GetPrimarySelectedNode(const Graph &graph) const;

    // View navigation
    void CenterViewOn(const TEVector2 &worldPos);
    void ZoomToFit(const Graph &graph);
    float GetZoom() const { return m_Zoom; }
    const TEVector2 &GetScrollOffset() const { return m_ScrollOffset; }

    // Coordinate conversions
    TEVector2 WorldToScreen(const TEVector2 &worldPos, const TEVector2 &screenOrigin) const;
    TEVector2 ScreenToWorld(const TEVector2 &screenPos, const TEVector2 &screenOrigin) const;

private:
    TEVector2 GetPinScreenPos(const Graph &graph, uint64_t pinId, const TEVector2 &origin) const;

    NodeCanvasStyle m_Style;
    NodePalettePopup m_Palette;
    NodeBodyDrawerFn m_CustomBodyDrawer = nullptr;

    TEVector2 m_ScrollOffset = {0.0f, 0.0f};
    float m_Zoom = 1.0f;

    TESet<uint64_t> m_SelectedNodeIDs;
    uint64_t m_DraggingNodeID = 0;
    bool m_IsDraggingNodes = false;
    TEVector2 m_DragDelta = {0.0f, 0.0f};

    // Pin dragging
    uint64_t m_ConnectingPinID = 0;
    bool m_IsConnectingPin = false;

    // Box selection
    bool m_IsBoxSelecting = false;
    TEVector2 m_BoxSelectStart = {0.0f, 0.0f};
    TEVector2 m_BoxSelectEnd = {0.0f, 0.0f};
};

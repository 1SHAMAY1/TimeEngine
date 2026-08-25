#pragma once

#include "Asset/DialogueTreeAsset.hpp"
#include "Editor/DialogueLiveSimulator.hpp"
#include "Editor/DialogueNodeInspector.hpp"
#include "Graph/DialogueGraph.hpp"
#include "Graph/DialogueGraphNode.hpp"
#include "Utils/MathUtils.hpp"
#include <cstdint>


struct PendingConnection
{
    uint64_t SourceNodeID = 0;
    uint64_t SourcePinID = 0;
    TEVector2 DragStart = {0.0f, 0.0f};
    bool bIsActive = false;
};

struct GraphUndoState
{
    TEString SerializedGraph;
};

class DialogueGraphCanvas
{
public:
    DialogueGraphCanvas() = default;
    ~DialogueGraphCanvas() = default;

    void Draw(DialogueGraph &graph, uint64_t highlightNodeID = 0);

    uint64_t GetSelectedNodeID() const { return m_SelectedNodeID; }
    void SetSelectedNodeID(uint64_t id) { m_SelectedNodeID = id; }

    void PushUndoState(const DialogueGraph &graph);
    bool CanUndo() const;
    void Undo(DialogueGraph &graph);

    bool IsDirty() const { return m_bIsDirty; }
    void ClearDirty() { m_bIsDirty = false; }

private:
    void DrawGrid(const TEVector2 &canvasPos, const TEVector2 &canvasSize);
    void DrawNode(DialogueGraph &graph, DialogueGraphNode &node, const TEVector2 &canvasOrigin,
                  float zoom, uint64_t highlightNodeID);
    void DrawConnections(const DialogueGraph &graph, const TEVector2 &canvasOrigin, float zoom);
    void DrawContextMenu(DialogueGraph &graph, const TEVector2 &canvasOrigin, float zoom);
    void DrawPendingLink(const TEVector2 &canvasOrigin, float zoom);

    TEVector2 WorldToCanvas(const TEVector2 &worldPos, const TEVector2 &canvasOrigin, float zoom) const;
    TEVector2 CanvasToWorld(const TEVector2 &screenPos, const TEVector2 &canvasOrigin, float zoom) const;
    TEVector2 GetPinScreenPos(const DialogueGraph &graph, uint64_t pinId, const TEVector2 &canvasOrigin,
                               float zoom) const;

    uint64_t m_SelectedNodeID = 0;
    TEVector2 m_ScrollOffset = {0.0f, 0.0f};
    float m_Zoom = 1.0f;
    bool m_bIsDraggingNode = false;
    bool m_bIsPanning = false;
    TEVector2 m_DragOffset = {0.0f, 0.0f};
    PendingConnection m_PendingLink;
    bool m_bIsDirty = false;
    bool m_bShowContextMenu = false;
    TEVector2 m_ContextMenuWorldPos = {0.0f, 0.0f};

    TEArray<GraphUndoState> m_UndoStack;
    static constexpr size_t s_MaxUndoDepth = 32;
};


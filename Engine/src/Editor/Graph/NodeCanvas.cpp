#include "Core/PreRequisites.h"
#include "Editor/Graph/NodeCanvas.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cmath>

NodeCanvas::NodeCanvas() {}

void NodeCanvas::SelectNode(uint64_t nodeId, bool clearOthers)
{
    if (clearOthers)
        m_SelectedNodeIDs.Clear();
    m_SelectedNodeIDs.Add(nodeId);
}

void NodeCanvas::ClearSelection() { m_SelectedNodeIDs.Clear(); }

TERef<GraphNode> NodeCanvas::GetPrimarySelectedNode(const Graph &graph) const
{
    if (m_SelectedNodeIDs.IsEmpty())
        return nullptr;
    auto it = m_SelectedNodeIDs.begin();
    return graph.FindNode(*it);
}

void NodeCanvas::CenterViewOn(const TEVector2 &worldPos)
{
    m_ScrollOffset = TEVector2(-worldPos.x * m_Zoom, -worldPos.y * m_Zoom);
}

void NodeCanvas::ZoomToFit(const Graph &graph)
{
    const auto &nodes = graph.GetNodes();
    if (nodes.IsEmpty())
    {
        m_ScrollOffset = {0.0f, 0.0f};
        m_Zoom = 1.0f;
        return;
    }

    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
    for (const auto &node : nodes)
    {
        minX = std::min(minX, node->Position.x);
        minY = std::min(minY, node->Position.y);
        maxX = std::max(maxX, node->Position.x + node->Size.x);
        maxY = std::max(maxY, node->Position.y + node->Size.y);
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    CenterViewOn(TEVector2(centerX, centerY));
}

TEVector2 NodeCanvas::WorldToScreen(const TEVector2 &worldPos, const TEVector2 &screenOrigin) const
{
    return TEVector2(screenOrigin.x + m_ScrollOffset.x + worldPos.x * m_Zoom,
                     screenOrigin.y + m_ScrollOffset.y + worldPos.y * m_Zoom);
}

TEVector2 NodeCanvas::ScreenToWorld(const TEVector2 &screenPos, const TEVector2 &screenOrigin) const
{
    return TEVector2((screenPos.x - screenOrigin.x - m_ScrollOffset.x) / m_Zoom,
                     (screenPos.y - screenOrigin.y - m_ScrollOffset.y) / m_Zoom);
}

TEVector2 NodeCanvas::GetPinScreenPos(const Graph &graph, uint64_t pinId, const TEVector2 &origin) const
{
    for (const auto &node : graph.GetNodes())
    {
        TEVector2 nodeScreenPos = WorldToScreen(node->Position, origin);
        float nodeWidth = node->Size.x * m_Zoom;
        float headerHeight = 28.0f * m_Zoom;
        float pinRowHeight = 22.0f * m_Zoom;

        // Check Input Pins (Left side)
        for (size_t i = 0; i < node->InputPins.Size(); ++i)
        {
            if (node->InputPins[i].ID == pinId)
            {
                float py = nodeScreenPos.y + headerHeight + (i + 0.5f) * pinRowHeight;
                return TEVector2(nodeScreenPos.x + 10.0f * m_Zoom, py);
            }
        }

        // Check Output Pins (Right side)
        for (size_t i = 0; i < node->OutputPins.Size(); ++i)
        {
            if (node->OutputPins[i].ID == pinId)
            {
                float py = nodeScreenPos.y + headerHeight + (i + 0.5f) * pinRowHeight;
                return TEVector2(nodeScreenPos.x + nodeWidth - 10.0f * m_Zoom, py);
            }
        }
    }
    return {0.0f, 0.0f};
}

void NodeCanvas::Draw(Graph &graph, const TEVector2 &canvasSize)
{
    TEVector2 availSize = canvasSize;
    if (availSize.x <= 0.0f || availSize.y <= 0.0f)
        availSize = TimeGUI::GetContentRegionAvail();

    TEVector2 canvasScreenPos = TimeGUI::GetCursorScreenPos();
    TimeGUI::InvisibleButton("##NodeCanvasArea", availSize);
    bool isCanvasHovered = TimeGUI::IsItemHovered();
    bool isCanvasActive = TimeGUI::IsItemActive();

    TEVector2 mousePos = TimeGUI::GetMousePos();
    TEVector2 worldMousePos = ScreenToWorld(mousePos, canvasScreenPos);

    // Zooming
    if (isCanvasHovered)
    {
        float mouseWheel = TimeGUI::GetIO().MouseWheel;
        if (mouseWheel != 0.0f)
        {
            float prevZoom = m_Zoom;
            m_Zoom = std::clamp(m_Zoom + mouseWheel * 0.1f, 0.25f, 2.5f);

            // Zoom towards mouse cursor
            TEVector2 mouseRel = mousePos - canvasScreenPos - m_ScrollOffset;
            m_ScrollOffset = m_ScrollOffset - mouseRel * ((m_Zoom - prevZoom) / prevZoom);
        }
    }

    // Panning (Middle Mouse Drag or Alt + Left Mouse Drag)
    if (isCanvasActive && (TimeGUI::IsMouseDragging(2) || (TimeGUI::GetIO().KeyAlt && TimeGUI::IsMouseDragging(0))))
    {
        TEVector2 dragDelta = TimeGUI::GetIO().MouseDelta;
        m_ScrollOffset = m_ScrollOffset + dragDelta;
    }

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Background and Grid
    dl.AddRectFilled(canvasScreenPos, canvasScreenPos + availSize, TimeGUI::GetColorU32(m_Style.BackgroundColor));

    // Draw Background Grid
    if (m_Zoom > 0.15f)
    {
        float scaledGrid = m_Style.GridSpacing * m_Zoom;
        float startX = std::fmod(m_ScrollOffset.x, scaledGrid);
        float startY = std::fmod(m_ScrollOffset.y, scaledGrid);

        unsigned int gridCol = TimeGUI::GetColorU32(m_Style.GridColor);

        for (float x = startX; x < availSize.x; x += scaledGrid)
        {
            dl.AddLine(TEVector2(canvasScreenPos.x + x, canvasScreenPos.y),
                       TEVector2(canvasScreenPos.x + x, canvasScreenPos.y + availSize.y), gridCol, 1.0f);
        }

        for (float y = startY; y < availSize.y; y += scaledGrid)
        {
            dl.AddLine(TEVector2(canvasScreenPos.x, canvasScreenPos.y + y),
                       TEVector2(canvasScreenPos.x + availSize.x, canvasScreenPos.y + y), gridCol, 1.0f);
        }
    }

    // Draw Connections (Bezier curves)
    for (const auto &conn : graph.GetConnections())
    {
        TEVector2 p1 = GetPinScreenPos(graph, conn.SourcePinID, canvasScreenPos);
        TEVector2 p2 = GetPinScreenPos(graph, conn.TargetPinID, canvasScreenPos);

        GraphPin *srcPin = graph.FindPin(conn.SourcePinID);
        TEColor pinColor = srcPin ? srcPin->Type.GetColor() : TEColor(0.8f, 0.8f, 0.8f, 1.0f);

        float dx = std::abs(p2.x - p1.x) * 0.5f;
        dx = std::max(dx, 40.0f * m_Zoom);

        TEVector2 cp1 = p1 + TEVector2(dx, 0.0f);
        TEVector2 cp2 = p2 - TEVector2(dx, 0.0f);

        dl.AddBezierCubic(p1, cp1, cp2, p2, TimeGUI::GetColorU32(pinColor), m_Style.WireThickness * m_Zoom);
    }

    // Draw Active Pin Dragging Connection
    if (m_IsConnectingPin && m_ConnectingPinID != 0)
    {
        TEVector2 p1 = GetPinScreenPos(graph, m_ConnectingPinID, canvasScreenPos);
        GraphPin *srcPin = graph.FindPin(m_ConnectingPinID);
        TEColor pinColor = srcPin ? srcPin->Type.GetColor() : TEColor(1.0f, 1.0f, 1.0f, 1.0f);

        float dx = std::abs(mousePos.x - p1.x) * 0.5f;
        dx = std::max(dx, 30.0f * m_Zoom);

        TEVector2 cp1 = (srcPin && srcPin->Direction == PinDirection::Input) ? (p1 - TEVector2(dx, 0.0f))
                                                                             : (p1 + TEVector2(dx, 0.0f));
        TEVector2 cp2 = (srcPin && srcPin->Direction == PinDirection::Input) ? (mousePos + TEVector2(dx, 0.0f))
                                                                             : (mousePos - TEVector2(dx, 0.0f));

        dl.AddBezierCubic(p1, cp1, cp2, mousePos, TimeGUI::GetColorU32(pinColor), m_Style.WireThickness * m_Zoom);

        if (TimeGUI::IsMouseReleased(0))
        {
            // Drop connection on candidate pin if any
            for (const auto &node : graph.GetNodes())
            {
                for (const auto &pin : node->InputPins)
                {
                    TEVector2 pinPos = GetPinScreenPos(graph, pin.ID, canvasScreenPos);
                    if ((mousePos - pinPos).Length() <= m_Style.PinRadius * m_Zoom * 2.0f)
                    {
                        graph.AddConnection(m_ConnectingPinID, pin.ID);
                        break;
                    }
                }
                for (const auto &pin : node->OutputPins)
                {
                    TEVector2 pinPos = GetPinScreenPos(graph, pin.ID, canvasScreenPos);
                    if ((mousePos - pinPos).Length() <= m_Style.PinRadius * m_Zoom * 2.0f)
                    {
                        graph.AddConnection(m_ConnectingPinID, pin.ID);
                        break;
                    }
                }
            }
            m_IsConnectingPin = false;
            m_ConnectingPinID = 0;
        }
    }

    // Draw Nodes
    uint64_t hoveredNodeID = 0;
    uint64_t clickedPinID = 0;

    for (const auto &node : graph.GetNodes())
    {
        TEVector2 nodeMin = WorldToScreen(node->Position, canvasScreenPos);
        float nodeWidth = node->Size.x * m_Zoom;
        float headerHeight = 28.0f * m_Zoom;
        size_t maxPins = std::max(node->InputPins.Size(), node->OutputPins.Size());
        float bodyHeight = std::max(node->Size.y * m_Zoom, headerHeight + (maxPins * 22.0f + 16.0f) * m_Zoom);
        TEVector2 nodeMax = nodeMin + TEVector2(nodeWidth, bodyHeight);

        bool isSelected = m_SelectedNodeIDs.Contains(node->ID);
        bool isNodeHovered =
            mousePos.x >= nodeMin.x && mousePos.x <= nodeMax.x && mousePos.y >= nodeMin.y && mousePos.y <= nodeMax.y;

        if (isNodeHovered && isCanvasHovered)
            hoveredNodeID = node->ID;

        // Node Background
        dl.AddRectFilled(nodeMin, nodeMax, TimeGUI::GetColorU32(m_Style.NodeBgColor), m_Style.NodeRounding * m_Zoom);

        // Header
        TEVector2 headerMax = TEVector2(nodeMax.x, nodeMin.y + headerHeight);
        dl.AddRectFilled(nodeMin, headerMax, TimeGUI::GetColorU32(node->HeaderColor), m_Style.NodeRounding * m_Zoom);

        // Header Title
        dl.AddText(nodeMin + TEVector2(10.0f * m_Zoom, 6.0f * m_Zoom), TIMEGUI_COL32_WHITE, node->Title);

        // Border
        TEColor borderColor = isSelected ? m_Style.NodeSelectedBorderColor : m_Style.NodeBorderColor;
        float borderWidth = isSelected ? 2.5f : m_Style.NodeBorderWidth;
        dl.AddRect(nodeMin, nodeMax, TimeGUI::GetColorU32(borderColor), m_Style.NodeRounding * m_Zoom, 0, borderWidth);

        // Input Pins
        float pinRowHeight = 22.0f * m_Zoom;
        for (size_t i = 0; i < node->InputPins.Size(); ++i)
        {
            const auto &pin = node->InputPins[i];
            float py = nodeMin.y + headerHeight + (i + 0.5f) * pinRowHeight;
            TEVector2 pinCenter(nodeMin.x + 10.0f * m_Zoom, py);

            bool isPinHovered = (mousePos - pinCenter).Length() <= m_Style.PinRadius * m_Zoom * 1.5f;
            if (isPinHovered && isCanvasHovered && TimeGUI::IsMouseClicked(0))
            {
                clickedPinID = pin.ID;
            }

            TEColor pinCol = pin.Type.GetColor();
            dl.AddCircleFilled(pinCenter, m_Style.PinRadius * m_Zoom, TimeGUI::GetColorU32(pinCol));

            dl.AddText(pinCenter + TEVector2(12.0f * m_Zoom, -6.0f * m_Zoom), TIMEGUI_COL32(220, 220, 220, 255),
                       pin.Name);
        }

        // Output Pins
        for (size_t i = 0; i < node->OutputPins.Size(); ++i)
        {
            const auto &pin = node->OutputPins[i];
            float py = nodeMin.y + headerHeight + (i + 0.5f) * pinRowHeight;
            TEVector2 pinCenter(nodeMax.x - 10.0f * m_Zoom, py);

            bool isPinHovered = (mousePos - pinCenter).Length() <= m_Style.PinRadius * m_Zoom * 1.5f;
            if (isPinHovered && isCanvasHovered && TimeGUI::IsMouseClicked(0))
            {
                clickedPinID = pin.ID;
            }

            TEColor pinCol = pin.Type.GetColor();
            dl.AddCircleFilled(pinCenter, m_Style.PinRadius * m_Zoom, TimeGUI::GetColorU32(pinCol));

            float textOffset = (float)(pin.Name.length() * 7) * m_Zoom + 14.0f * m_Zoom;
            dl.AddText(pinCenter - TEVector2(textOffset, 6.0f * m_Zoom), TIMEGUI_COL32(220, 220, 220, 255), pin.Name);
        }
    }

    // Handle Pin Click to Start Connection
    if (clickedPinID != 0)
    {
        m_IsConnectingPin = true;
        m_ConnectingPinID = clickedPinID;
    }

    // Node Dragging & Selection Handling
    if (isCanvasHovered && TimeGUI::IsMouseClicked(0) && !m_IsConnectingPin)
    {
        if (hoveredNodeID != 0)
        {
            bool isShiftDown = TimeGUI::GetIO().KeyShift;
            if (isShiftDown)
            {
                if (m_SelectedNodeIDs.Contains(hoveredNodeID))
                    m_SelectedNodeIDs.Remove(hoveredNodeID);
                else
                    m_SelectedNodeIDs.Add(hoveredNodeID);
            }
            else if (!m_SelectedNodeIDs.Contains(hoveredNodeID))
            {
                SelectNode(hoveredNodeID, true);
            }

            m_DraggingNodeID = hoveredNodeID;
            m_IsDraggingNodes = true;
        }
        else
        {
            ClearSelection();
            m_IsBoxSelecting = true;
            m_BoxSelectStart = mousePos;
            m_BoxSelectEnd = mousePos;
        }
    }

    if (m_IsDraggingNodes)
    {
        if (TimeGUI::IsMouseDragging(0))
        {
            TEVector2 dragDelta = TimeGUI::GetIO().MouseDelta;
            TEVector2 worldDelta = dragDelta / m_Zoom;

            for (uint64_t id : m_SelectedNodeIDs)
            {
                auto node = graph.FindNode(id);
                if (node)
                {
                    node->Position = node->Position + worldDelta;
                }
            }
        }
        if (TimeGUI::IsMouseReleased(0))
        {
            m_IsDraggingNodes = false;
            m_DraggingNodeID = 0;
        }
    }

    // Box Selection Drag
    if (m_IsBoxSelecting)
    {
        m_BoxSelectEnd = mousePos;
        TEVector2 bMin(std::min(m_BoxSelectStart.x, m_BoxSelectEnd.x), std::min(m_BoxSelectStart.y, m_BoxSelectEnd.y));
        TEVector2 bMax(std::max(m_BoxSelectStart.x, m_BoxSelectEnd.x), std::max(m_BoxSelectStart.y, m_BoxSelectEnd.y));

        dl.AddRectFilled(bMin, bMax, TIMEGUI_COL32(60, 140, 240, 40));
        dl.AddRect(bMin, bMax, TIMEGUI_COL32(80, 160, 255, 200), 0.0f, 0, 1.0f);

        if (TimeGUI::IsMouseReleased(0))
        {
            m_IsBoxSelecting = false;
            for (const auto &node : graph.GetNodes())
            {
                TEVector2 nPos = WorldToScreen(node->Position, canvasScreenPos);
                if (nPos.x >= bMin.x && nPos.x <= bMax.x && nPos.y >= bMin.y && nPos.y <= bMax.y)
                {
                    m_SelectedNodeIDs.Add(node->ID);
                }
            }
        }
    }

    // Deletion
    if (isCanvasHovered &&
        (TimeGUI::IsKeyPressed(TimeGUI::TimeGUIKey_Delete) || TimeGUI::IsKeyPressed(TimeGUI::TimeGUIKey_Backspace)))
    {
        for (uint64_t id : m_SelectedNodeIDs)
        {
            graph.RemoveNode(id);
        }
        m_SelectedNodeIDs.Clear();
    }

    // Open Node Palette on Right-Click
    if (isCanvasHovered && TimeGUI::IsMouseClicked(1))
    {
        m_Palette.Open(worldMousePos);
    }

    // Draw Palette
    auto spawnedNode = m_Palette.Draw();
    if (spawnedNode)
    {
        graph.AddNode(spawnedNode);
        SelectNode(spawnedNode->ID, true);
    }
}

#include "DialogueGraphCanvas.hpp"
#include "Asset/DialogueTreeSerializer.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>


// ── Node type colour palette ──────────────────────────────────────────────────
static unsigned int GetNodeHeaderColor(NarrativeNodeType type)
{
    switch (type)
    {
    case NarrativeNodeType::Entry:    return TimeGUI::ColorConvertFloat4ToU32(TEVector4(80.0f/255.0f,  160.0f/255.0f, 80.0f/255.0f,  1.0f)); // Green
    case NarrativeNodeType::Dialogue: return TimeGUI::ColorConvertFloat4ToU32(TEVector4(55.0f/255.0f,  110.0f/255.0f, 195.0f/255.0f, 1.0f)); // Blue
    case NarrativeNodeType::Choice:   return TimeGUI::ColorConvertFloat4ToU32(TEVector4(180.0f/255.0f, 130.0f/255.0f, 40.0f/255.0f,  1.0f)); // Amber
    case NarrativeNodeType::Condition:return TimeGUI::ColorConvertFloat4ToU32(TEVector4(155.0f/255.0f, 80.0f/255.0f,  190.0f/255.0f, 1.0f)); // Purple
    case NarrativeNodeType::Action:   return TimeGUI::ColorConvertFloat4ToU32(TEVector4(190.0f/255.0f, 70.0f/255.0f,  60.0f/255.0f,  1.0f)); // Red
    case NarrativeNodeType::Divert:   return TimeGUI::ColorConvertFloat4ToU32(TEVector4(60.0f/255.0f,  160.0f/255.0f, 170.0f/255.0f, 1.0f)); // Teal
    case NarrativeNodeType::Exit:     return TimeGUI::ColorConvertFloat4ToU32(TEVector4(100.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f)); // Grey
    default:                          return TimeGUI::ColorConvertFloat4ToU32(TEVector4(80.0f/255.0f,  80.0f/255.0f,  80.0f/255.0f,  1.0f));
    }
}

static const char *NodeTypeIcon(NarrativeNodeType type)
{
    switch (type)
    {
    case NarrativeNodeType::Entry:     return "▶  Entry";
    case NarrativeNodeType::Dialogue:  return "💬 Dialogue";
    case NarrativeNodeType::Choice:    return "⚡ Choices";
    case NarrativeNodeType::Condition: return "❓ Condition";
    case NarrativeNodeType::Action:    return "⚙  Action";
    case NarrativeNodeType::Divert:    return "→  Divert";
    case NarrativeNodeType::Exit:      return "■  Exit";
    default:                           return "Node";
    }
}

// ── Coordinate helpers ────────────────────────────────────────────────────────
TEVector2 DialogueGraphCanvas::WorldToCanvas(const TEVector2 &wp, const TEVector2 &origin, float zoom) const
{
    return TEVector2(origin.x + m_ScrollOffset.x + wp.x * zoom,
                     origin.y + m_ScrollOffset.y + wp.y * zoom);
}

TEVector2 DialogueGraphCanvas::CanvasToWorld(const TEVector2 &sp, const TEVector2 &origin, float zoom) const
{
    return TEVector2((sp.x - origin.x - m_ScrollOffset.x) / zoom,
                     (sp.y - origin.y - m_ScrollOffset.y) / zoom);
}

static constexpr float k_PinRadius  = 6.0f;
static constexpr float k_PinOffsetX = 10.0f;

TEVector2 DialogueGraphCanvas::GetPinScreenPos(const DialogueGraph &graph, uint64_t pinId,
                                               const TEVector2 &origin, float zoom) const
{
    for (size_t n = 0; n < graph.GetNodes().Size(); ++n)
    {
        const auto &node = graph.GetNodes()[n];
        TEVector2 nodeScreen = WorldToCanvas(node.Position, origin, zoom);

        float headerH = 22.0f * zoom;
        float rowH    = 20.0f * zoom;

        for (size_t p = 0; p < node.InputPins.Size(); ++p)
        {
            if (node.InputPins[p].ID == pinId)
            {
                float y = nodeScreen.y + headerH + p * rowH + rowH * 0.5f;
                return TEVector2(nodeScreen.x - k_PinOffsetX, y);
            }
        }
        for (size_t p = 0; p < node.OutputPins.Size(); ++p)
        {
            if (node.OutputPins[p].ID == pinId)
            {
                float y = nodeScreen.y + headerH + p * rowH + rowH * 0.5f;
                return TEVector2(nodeScreen.x + node.Size.x * zoom + k_PinOffsetX, y);
            }
        }
    }
    return TEVector2(0.0f, 0.0f);
}

// ── Grid ─────────────────────────────────────────────────────────────────────
void DialogueGraphCanvas::DrawGrid(const TEVector2 &canvasPos, const TEVector2 &canvasSize)
{
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    unsigned int minor = TimeGUI::ColorConvertFloat4ToU32(TEVector4(55.0f/255.0f, 55.0f/255.0f, 60.0f/255.0f, 1.0f));
    unsigned int major = TimeGUI::ColorConvertFloat4ToU32(TEVector4(75.0f/255.0f, 75.0f/255.0f, 85.0f/255.0f, 1.0f));

    float gridMinor = 20.0f * m_Zoom;
    float gridMajor = 100.0f * m_Zoom;

    for (float x = fmodf(m_ScrollOffset.x, gridMinor); x < canvasSize.x; x += gridMinor)
    {
        bool isMaj = fmodf(x + canvasSize.x, gridMajor) < gridMinor;
        dl.AddLine(TEVector2(canvasPos.x + x, canvasPos.y),
                   TEVector2(canvasPos.x + x, canvasPos.y + canvasSize.y),
                   isMaj ? major : minor, isMaj ? 1.5f : 0.8f);
    }
    for (float y = fmodf(m_ScrollOffset.y, gridMinor); y < canvasSize.y; y += gridMinor)
    {
        bool isMaj = fmodf(y + canvasSize.y, gridMajor) < gridMinor;
        dl.AddLine(TEVector2(canvasPos.x, canvasPos.y + y),
                   TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + y),
                   isMaj ? major : minor, isMaj ? 1.5f : 0.8f);
    }
}

// ── Bezier spline helper ──────────────────────────────────────────────────────
static void DrawBezier(TimeGUIDrawList &dl, TEVector2 p0, TEVector2 p1, unsigned int col, float thickness)
{
    float dx = fabsf(p1.x - p0.x) * 0.5f;
    dx = std::max(dx, 50.0f);
    TEVector2 cp0(p0.x + dx, p0.y);
    TEVector2 cp1(p1.x - dx, p1.y);
    dl.AddBezierCubic(p0, cp0, cp1, p1, col, thickness, 24);
}

// ── Connections ───────────────────────────────────────────────────────────────
void DialogueGraphCanvas::DrawConnections(const DialogueGraph &graph, const TEVector2 &origin, float zoom)
{
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    for (size_t i = 0; i < graph.GetConnections().Size(); ++i)
    {
        const auto &conn = graph.GetConnections()[i];
        TEVector2 from = GetPinScreenPos(graph, conn.SourcePinID, origin, zoom);
        TEVector2 to   = GetPinScreenPos(graph, conn.TargetPinID, origin, zoom);

        unsigned int col = TimeGUI::ColorConvertFloat4ToU32(TEVector4(160.0f/255.0f, 160.0f/255.0f, 200.0f/255.0f, 0.8f));

        const auto *srcNode = graph.FindNode(conn.SourceNodeID);
        if (srcNode)
        {
            col = GetNodeHeaderColor(srcNode->Type);
            col = (col & 0x00FFFFFF) | 0xCC000000;
        }

        DrawBezier(dl, from, to, col, 2.5f * zoom);
    }
}

// ── Pending link drag ─────────────────────────────────────────────────────────
void DialogueGraphCanvas::DrawPendingLink(const TEVector2 &origin, float zoom)
{
    if (!m_PendingLink.bIsActive)
        return;

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector2 mousePos = TimeGUI::GetMousePos();
    DrawBezier(dl,
               m_PendingLink.DragStart,
               mousePos,
               TimeGUI::ColorConvertFloat4ToU32(TEVector4(200.0f/255.0f, 200.0f/255.0f, 100.0f/255.0f, 0.8f)),
               2.0f * zoom);
}

// ── Single node ───────────────────────────────────────────────────────────────
void DialogueGraphCanvas::DrawNode(DialogueGraph &graph, DialogueGraphNode &node,
                                   const TEVector2 &origin, float zoom, uint64_t highlightNodeID)
{
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    TEVector2 nodeScreen = WorldToCanvas(node.Position, origin, zoom);
    float w = node.Size.x * zoom;
    float h = node.Size.y * zoom;

    TEVector2 tl(nodeScreen.x, nodeScreen.y);
    TEVector2 br(nodeScreen.x + w, nodeScreen.y + h);

    bool isSelected   = (node.ID == m_SelectedNodeID);
    bool isHighlighted = (highlightNodeID != 0 && node.ID == highlightNodeID);

    // Shadow
    dl.AddRectFilled(TEVector2(tl.x + 4, tl.y + 4), TEVector2(br.x + 4, br.y + 4),
                     TimeGUI::ColorConvertFloat4ToU32(TEVector4(0.0f, 0.0f, 0.0f, 0.35f)), 8.0f * zoom);

    // Body
    unsigned int bodyCol = TimeGUI::ColorConvertFloat4ToU32(TEVector4(38.0f/255.0f, 38.0f/255.0f, 45.0f/255.0f, 1.0f));
    dl.AddRectFilled(tl, br, bodyCol, 8.0f * zoom);

    // Selection / Highlight glow
    if (isHighlighted)
    {
        float alpha = 0.8f;
        dl.AddRect(TEVector2(tl.x - 3, tl.y - 3), TEVector2(br.x + 3, br.y + 3),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(80.0f/255.0f, 240.0f/255.0f, 100.0f/255.0f, alpha)),
                   10.0f * zoom, 0, 3.0f);
    }
    else if (isSelected)
    {
        dl.AddRect(TEVector2(tl.x - 2, tl.y - 2), TEVector2(br.x + 2, br.y + 2),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(220.0f/255.0f, 200.0f/255.0f, 60.0f/255.0f, 1.0f)),
                   10.0f * zoom, 0, 2.5f);
    }

    // Header
    float headerH = 22.0f * zoom;
    unsigned int headerCol = GetNodeHeaderColor(node.Type);
    dl.AddRectFilled(tl, TEVector2(br.x, tl.y + headerH), headerCol, 8.0f * zoom);
    dl.AddRectFilled(TEVector2(tl.x, tl.y + headerH - 4), TEVector2(br.x, tl.y + headerH), headerCol, 0.0f);

    // Header text
    TEString headerText = node.Title.empty() ? NodeTypeIcon(node.Type) : node.Title;
    dl.AddText(TEVector2(tl.x + 6 * zoom, tl.y + 4 * zoom),
               TimeGUI::ColorConvertFloat4ToU32(TEVector4(240.0f/255.0f, 240.0f/255.0f, 240.0f/255.0f, 1.0f)),
               headerText);

    // Preview text (dialogue / speaker)
    float rowH = 20.0f * zoom;
    float contentY = tl.y + headerH + 4 * zoom;

    if (node.Type == NarrativeNodeType::Dialogue)
    {
        TEString speakerLine = node.Speaker.empty() ? "" : node.Speaker + ":";
        if (!speakerLine.empty())
        {
            dl.AddText(TEVector2(tl.x + 8 * zoom, contentY),
                       TimeGUI::ColorConvertFloat4ToU32(TEVector4(120.0f/255.0f, 190.0f/255.0f, 255.0f/255.0f, 0.85f)),
                       speakerLine);
            contentY += rowH * 0.8f;
        }
        TEString preview = node.Text.size() > 36 ? node.Text.substr(0, 36) + "…" : node.Text;
        dl.AddText(TEVector2(tl.x + 8 * zoom, contentY),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(210.0f/255.0f, 210.0f/255.0f, 210.0f/255.0f, 0.8f)),
                   preview);
    }
    else if (node.Type == NarrativeNodeType::Choice)
    {
        for (size_t c = 0; c < std::min((size_t)node.Choices.Size(), (size_t)3); ++c)
        {
            TEString line = "• " + (node.Choices[c].Text.size() > 28 ?
                               node.Choices[c].Text.substr(0, 28) + "…" : node.Choices[c].Text);
            dl.AddText(TEVector2(tl.x + 8 * zoom, contentY + c * rowH * 0.85f),
                       TimeGUI::ColorConvertFloat4ToU32(TEVector4(250.0f/255.0f, 215.0f/255.0f, 100.0f/255.0f, 0.8f)),
                       line);
        }
    }
    else if (node.Type == NarrativeNodeType::Condition)
    {
        TEString cond = node.ConditionVar + " ? " + node.ConditionValue.AsString();
        dl.AddText(TEVector2(tl.x + 8 * zoom, contentY),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(200.0f/255.0f, 160.0f/255.0f, 240.0f/255.0f, 0.8f)),
                   cond);
    }
    else if (node.Type == NarrativeNodeType::Action)
    {
        TEString action = node.ActionVar + " → " + node.ActionValue.AsString();
        dl.AddText(TEVector2(tl.x + 8 * zoom, contentY),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(255.0f/255.0f, 150.0f/255.0f, 130.0f/255.0f, 0.8f)),
                   action);
    }
    else if (node.Type == NarrativeNodeType::Divert)
    {
        dl.AddText(TEVector2(tl.x + 8 * zoom, contentY),
                   TimeGUI::ColorConvertFloat4ToU32(TEVector4(100.0f/255.0f, 220.0f/255.0f, 220.0f/255.0f, 0.8f)),
                   node.DivertTargetKnot);
    }

    // ── Pins ─────────────────────────────────────────────────────────────────
    float pinY = tl.y + headerH;

    // Input pins (left side)
    for (size_t p = 0; p < node.InputPins.Size(); ++p)
    {
        float cy = pinY + p * rowH + rowH * 0.5f;
        TEVector2 pinCenter(tl.x - k_PinOffsetX, cy);
        dl.AddCircleFilled(pinCenter, k_PinRadius * zoom,
                           TimeGUI::ColorConvertFloat4ToU32(TEVector4(80.0f/255.0f, 160.0f/255.0f, 80.0f/255.0f, 1.0f)));
        dl.AddCircle(pinCenter, k_PinRadius * zoom,
                     TimeGUI::ColorConvertFloat4ToU32(TEVector4(200.0f/255.0f, 220.0f/255.0f, 200.0f/255.0f, 1.0f)),
                     12, 1.5f);
    }

    // Output pins (right side)
    for (size_t p = 0; p < node.OutputPins.Size(); ++p)
    {
        float cy = pinY + p * rowH + rowH * 0.5f;
        TEVector2 pinCenter(br.x + k_PinOffsetX, cy);
        unsigned int pinCol = node.OutputPins[p].Type.GetKind() == GraphPinKind::Custom
                           ? TimeGUI::ColorConvertFloat4ToU32(TEVector4(230.0f/255.0f, 180.0f/255.0f, 40.0f/255.0f, 1.0f))
                           : TimeGUI::ColorConvertFloat4ToU32(TEVector4(70.0f/255.0f, 130.0f/255.0f, 210.0f/255.0f, 1.0f));
        dl.AddCircleFilled(pinCenter, k_PinRadius * zoom, pinCol);
        dl.AddCircle(pinCenter, k_PinRadius * zoom,
                     TimeGUI::ColorConvertFloat4ToU32(TEVector4(200.0f/255.0f, 200.0f/255.0f, 230.0f/255.0f, 1.0f)),
                     12, 1.5f);

        const TEString &pName = node.OutputPins[p].Name;
        if (!pName.empty() && node.OutputPins.Size() > 1)
        {
            dl.AddText(TEVector2(br.x + k_PinOffsetX + 9 * zoom, cy - 7 * zoom),
                       TimeGUI::ColorConvertFloat4ToU32(TEVector4(180.0f/255.0f, 180.0f/255.0f, 180.0f/255.0f, 0.8f)),
                       pName);
        }
    }

    // ── Hit-testing for interaction ────────────────────────────────────────
    TimeGUI::SetCursorScreenPos(tl);
    TimeGUI::PushID(static_cast<int>(node.ID));
    TimeGUI::InvisibleButton("##node", TEVector2(w, h));

    bool nodeHovered = TimeGUI::IsItemHovered();
    bool nodeActive  = TimeGUI::IsItemActive();

    // Selection
    if (TimeGUI::IsItemClicked(0))
    {
        if (m_SelectedNodeID != node.ID)
        {
            PushUndoState(graph);
            m_SelectedNodeID = node.ID;
        }
    }

    // Dragging
    if (nodeActive && TimeGUI::IsMouseDragging(0))
    {
        TEVector2 delta = TimeGUI::GetMouseDragDelta(0);
        node.Position.x += delta.x / zoom;
        node.Position.y += delta.y / zoom;
        TimeGUI::ResetMouseDragDelta(0);
        m_bIsDirty = true;
    }

    // Tooltip
    if (nodeHovered)
    {
        TimeGUI::BeginTooltip();
        TimeGUI::Text("%s (ID: %llu)", NodeTypeIcon(node.Type), node.ID);
        if (!node.Speaker.empty())
            TimeGUI::Text("Speaker: %s", node.Speaker.c_str());
        if (!node.Text.empty())
            TimeGUI::TextWrapped("%s", node.Text.c_str());
        TimeGUI::EndTooltip();
    }

    // Output pin dragging
    for (size_t p = 0; p < node.OutputPins.Size(); ++p)
    {
        float cy = tl.y + headerH + p * rowH + rowH * 0.5f;
        TEVector2 pinCenter(br.x + k_PinOffsetX, cy);
        TEVector2 mousePos = TimeGUI::GetMousePos();
        float dist = sqrtf(powf(mousePos.x - pinCenter.x, 2) +
                           powf(mousePos.y - pinCenter.y, 2));
        if (dist < k_PinRadius * zoom * 2.5f && TimeGUI::IsMouseClicked(0))
        {
            m_PendingLink.bIsActive   = true;
            m_PendingLink.SourceNodeID = node.ID;
            m_PendingLink.SourcePinID  = node.OutputPins[p].ID;
            m_PendingLink.DragStart    = pinCenter;
        }
    }

    // Input pin drop
    if (m_PendingLink.bIsActive && TimeGUI::IsMouseReleased(0))
    {
        for (size_t p = 0; p < node.InputPins.Size(); ++p)
        {
            float cy = tl.y + headerH + p * rowH + rowH * 0.5f;
            TEVector2 pinCenter(tl.x - k_PinOffsetX, cy);
            TEVector2 mousePos = TimeGUI::GetMousePos();
            float dist = sqrtf(powf(mousePos.x - pinCenter.x, 2) +
                               powf(mousePos.y - pinCenter.y, 2));
            if (dist < k_PinRadius * zoom * 3.5f && node.ID != m_PendingLink.SourceNodeID)
            {
                PushUndoState(graph);
                graph.Connect(m_PendingLink.SourceNodeID, m_PendingLink.SourcePinID,
                              node.ID, node.InputPins[p].ID);
                m_bIsDirty = true;
                break;
            }
        }
        m_PendingLink.bIsActive = false;
    }

    TimeGUI::PopID();
}

// ── Context Menu ──────────────────────────────────────────────────────────────
void DialogueGraphCanvas::DrawContextMenu(DialogueGraph &graph, const TEVector2 &origin, float zoom)
{
    if (!m_bShowContextMenu)
        return;

    TimeGUI::OpenPopup("##GraphContextMenu");
    m_bShowContextMenu = false;

    if (TimeGUI::BeginPopup("##GraphContextMenu"))
    {
        TimeGUI::Text("Add Node");
        TimeGUI::Separator();

        struct NodeEntry { const TEString& label; NarrativeNodeType type; };
        NodeEntry entries[] = {
            { "💬 Dialogue Line",     NarrativeNodeType::Dialogue  },
            { "⚡ Branching Choices", NarrativeNodeType::Choice    },
            { "❓ Condition Check",   NarrativeNodeType::Condition },
            { "⚙  Action / Event",   NarrativeNodeType::Action    },
            { "→  Divert / Jump",     NarrativeNodeType::Divert    },
            { "■  Exit Conversation", NarrativeNodeType::Exit      },
        };

        for (const auto &e : entries)
        {
            if (TimeGUI::MenuItem(e.label))
            {
                PushUndoState(graph);
                graph.AddNode(e.type, m_ContextMenuWorldPos);
                m_bIsDirty = true;
            }
        }

        TimeGUI::Separator();
        if (m_SelectedNodeID != 0 && TimeGUI::MenuItem("🗑  Delete Selected Node"))
        {
            PushUndoState(graph);
            graph.RemoveNode(m_SelectedNodeID);
            m_SelectedNodeID = 0;
            m_bIsDirty = true;
        }

        TimeGUI::EndPopup();
    }
}

// ── Main Draw ─────────────────────────────────────────────────────────────────
void DialogueGraphCanvas::Draw(DialogueGraph &graph, uint64_t highlightNodeID)
{
    TEVector2 canvasPos  = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
    if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;

    // Background
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    dl.AddRectFilled(canvasPos,
                     TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                     TimeGUI::ColorConvertFloat4ToU32(TEVector4(28.0f/255.0f, 28.0f/255.0f, 33.0f/255.0f, 1.0f)));

    DrawGrid(canvasPos, canvasSize);
    DrawConnections(graph, canvasPos, m_Zoom);
    DrawPendingLink(canvasPos, m_Zoom);

    // Cancel pending link
    if (m_PendingLink.bIsActive && TimeGUI::IsMouseClicked(1))
    {
        m_PendingLink.bIsActive = false;
    }
    if (m_PendingLink.bIsActive && TimeGUI::IsMouseReleased(0))
    {
        m_PendingLink.bIsActive = false;
    }

    // Draw nodes with clip rect
    TimeGUI::SetCursorScreenPos(canvasPos);
    dl.PushClipRect(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);
    for (size_t i = 0; i < graph.GetNodes().Size(); ++i)
    {
        DrawNode(graph, graph.GetNodes()[i], canvasPos, m_Zoom, highlightNodeID);
    }
    dl.PopClipRect();

    // Canvas interaction: pan and zoom
    TimeGUI::SetCursorScreenPos(canvasPos);
    TimeGUI::InvisibleButton("##canvas_bg", canvasSize);

    // Zoom
    if (TimeGUI::IsItemHovered())
    {
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            float zoomDelta = wheel * 0.1f * m_Zoom;
            m_Zoom = std::max(0.2f, std::min(m_Zoom + zoomDelta, 2.5f));
        }
    }

    // Pan
    if (TimeGUI::IsItemActive())
    {
        bool doingPan = TimeGUI::IsMouseDragging(2) || (TimeGUI::IsMouseDragging(0) && TimeGUI::GetIO().KeyAlt);
        if (doingPan)
        {
            TEVector2 delta = TimeGUI::GetMouseDragDelta(2);
            if (delta.x == 0.0f && delta.y == 0.0f)
                delta = TimeGUI::GetMouseDragDelta(0);
            m_ScrollOffset.x += delta.x;
            m_ScrollOffset.y += delta.y;
            TimeGUI::ResetMouseDragDelta(2);
            TimeGUI::ResetMouseDragDelta(0);
        }

        // Right click on canvas
        if (TimeGUI::IsMouseReleased(1))
        {
            TEVector2 mousePos = TimeGUI::GetMousePos();
            m_ContextMenuWorldPos = CanvasToWorld(mousePos, canvasPos, m_Zoom);
            m_bShowContextMenu = true;
        }

        // Deselect
        if (TimeGUI::IsMouseClicked(0) && !m_bShowContextMenu)
        {
            m_SelectedNodeID = 0;
        }
    }

    DrawContextMenu(graph, canvasPos, m_Zoom);
}

// ── Undo ──────────────────────────────────────────────────────────────────────
void DialogueGraphCanvas::PushUndoState(const DialogueGraph &graph)
{
    if (m_UndoStack.size() >= s_MaxUndoDepth)
        m_UndoStack.erase(m_UndoStack.begin());
    GraphUndoState state;
    state.SerializedGraph = DialogueTreeSerializer::SerializeToNativeText(graph);
    m_UndoStack.push_back(std::move(state));
}

bool DialogueGraphCanvas::CanUndo() const
{
    return !m_UndoStack.empty();
}

void DialogueGraphCanvas::Undo(DialogueGraph &graph)
{
    if (m_UndoStack.empty())
        return;
    GraphUndoState state = m_UndoStack.back();
    m_UndoStack.pop_back();
    DialogueTreeSerializer::DeserializeFromNativeText(graph, state.SerializedGraph);
    m_bIsDirty = true;
}


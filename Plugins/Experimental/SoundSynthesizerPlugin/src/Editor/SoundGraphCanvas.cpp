#include "SoundGraphCanvas.hpp"
#include "../Graph/SoundNodeRegistry.hpp"

namespace SoundStudio
{

static inline unsigned int ColorToU32(const TEColor &c)
{
    unsigned int r = static_cast<unsigned int>(std::clamp(c.r, 0.0f, 1.0f) * 255.0f);
    unsigned int g = static_cast<unsigned int>(std::clamp(c.g, 0.0f, 1.0f) * 255.0f);
    unsigned int b = static_cast<unsigned int>(std::clamp(c.b, 0.0f, 1.0f) * 255.0f);
    unsigned int a = static_cast<unsigned int>(std::clamp(c.a, 0.0f, 1.0f) * 255.0f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

SoundGraphCanvas::SoundGraphCanvas() {}

void SoundGraphCanvas::Draw(TERef<SoundGraph> graph, float width, float height)
{
    if (!graph)
        return;

    TimeGUI::BeginChild("SoundGraphCanvasRegion", TEVector2(width, height), true,
                        TimeGUIWindowFlags_NoScrollbar | TimeGUIWindowFlags_NoMove);

    // Canvas Background
    TEVector2 canvasP0 = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasP1 = {canvasP0.x + width, canvasP0.y + height};

    // Draw Grid
    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    float GRID_STEP = 32.0f;
    for (float x = std::fmod(m_Scrolling.x, GRID_STEP); x < width; x += GRID_STEP)
        drawList.AddLine({canvasP0.x + x, canvasP0.y}, {canvasP0.x + x, canvasP1.y},
                         ColorToU32(TEColor(0.2f, 0.22f, 0.26f, 0.5f)));
    for (float y = std::fmod(m_Scrolling.y, GRID_STEP); y < height; y += GRID_STEP)
        drawList.AddLine({canvasP0.x, canvasP0.y + y}, {canvasP1.x, canvasP0.y + y},
                         ColorToU32(TEColor(0.2f, 0.22f, 0.26f, 0.5f)));

    DrawConnections(graph);
    DrawNodes(graph);
    DrawContextMenu(graph);

    TimeGUI::EndChild();
}

void SoundGraphCanvas::DrawConnections(TERef<SoundGraph> graph)
{
    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    TEVector2 origin = TimeGUI::GetCursorScreenPos();

    for (const auto &conn : graph->GetConnections())
    {
        SoundPin *pOut = graph->FindPin(conn.OutputPinID);
        SoundPin *pIn = graph->FindPin(conn.InputPinID);

        if (pOut && pIn)
        {
            auto nodeOut = graph->GetNode(pOut->NodeID);
            auto nodeIn = graph->GetNode(pIn->NodeID);

            if (nodeOut && nodeIn)
            {
                TEVector2 posOut = {origin.x + nodeOut->GetPosition().x + 160.0f,
                                    origin.y + nodeOut->GetPosition().y + 35.0f};
                TEVector2 posIn = {origin.x + nodeIn->GetPosition().x, origin.y + nodeIn->GetPosition().y + 35.0f};

                TEVector2 cp1 = {posOut.x + 50.0f, posOut.y};
                TEVector2 cp2 = {posIn.x - 50.0f, posIn.y};

                drawList.AddBezierCubic(posOut, cp1, cp2, posIn, ColorToU32(pOut->GetPinColor()), 3.0f);
            }
        }
    }
}

void SoundGraphCanvas::DrawNodes(TERef<SoundGraph> graph)
{
    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    TEVector2 origin = TimeGUI::GetCursorScreenPos();

    for (auto &pair : graph->GetNodes())
    {
        auto node = pair.second;
        TimeGUI::PushID((int)node->GetID());

        TEVector2 nodePos = {origin.x + node->GetPosition().x, origin.y + node->GetPosition().y};
        TEVector2 nodeSize = {170.0f, 120.0f};

        // Node card background
        drawList.AddRectFilled(nodePos, {nodePos.x + nodeSize.x, nodePos.y + nodeSize.y},
                               ColorToU32(TEColor(0.16f, 0.18f, 0.22f, 0.95f)), 6.0f);
        drawList.AddRect(nodePos, {nodePos.x + nodeSize.x, nodePos.y + nodeSize.y},
                         ColorToU32((m_SelectedNodeId == node->GetID()) ? TEColor(0.3f, 0.7f, 1.0f, 1.0f)
                                                                        : TEColor(0.28f, 0.3f, 0.35f, 1.0f)),
                         6.0f, 0, (m_SelectedNodeId == node->GetID()) ? 2.5f : 1.0f);

        // Header bar
        drawList.AddRectFilled(nodePos, {nodePos.x + nodeSize.x, nodePos.y + 26.0f},
                               ColorToU32(TEColor(0.22f, 0.25f, 0.32f, 1.0f)), 6.0f);
        drawList.AddText({nodePos.x + 8.0f, nodePos.y + 5.0f}, ColorToU32(TEColor::White()), node->GetTitle());

        // Draw Pins
        float inY = nodePos.y + 35.0f;
        for (auto &pin : node->GetInputs())
        {
            drawList.AddCircleFilled({nodePos.x + 6.0f, inY + 6.0f}, 4.5f, ColorToU32(pin.GetPinColor()));
            drawList.AddText({nodePos.x + 14.0f, inY}, ColorToU32(TEColor(0.85f, 0.85f, 0.85f, 1.0f)), pin.Name);
            inY += 18.0f;
        }

        float outY = nodePos.y + 35.0f;
        for (auto &pin : node->GetOutputs())
        {
            drawList.AddCircleFilled({nodePos.x + nodeSize.x - 6.0f, outY + 6.0f}, 4.5f, ColorToU32(pin.GetPinColor()));
            drawList.AddText({nodePos.x + nodeSize.x - 65.0f, outY}, ColorToU32(TEColor(0.85f, 0.85f, 0.85f, 1.0f)),
                             pin.Name);
            outY += 18.0f;
        }

        TimeGUI::PopID();
    }
}

void SoundGraphCanvas::DrawContextMenu(TERef<SoundGraph> graph)
{
    if (TimeGUI::BeginPopupContextWindow("SoundGraphContextMenu", TimeGUIPopupFlags_MouseButtonRight))
    {
        TimeGUI::TextColored(TEVector4(0.3f, 0.85f, 0.95f, 1.0f), "Add Sound DSP Node:");
        TimeGUI::Separator();

        const auto &registry = SoundNodeRegistry::GetRegisteredNodes();
        for (const auto &pair : registry)
        {
            if (TimeGUI::MenuItem(pair.first.c_str()))
            {
                auto node = SoundNodeRegistry::CreateNode(pair.first);
                if (node)
                {
                    node->SetPosition({200.0f, 200.0f});
                    graph->AddNode(node);
                }
            }
        }
        TimeGUI::EndPopup();
    }
}

} // namespace SoundStudio

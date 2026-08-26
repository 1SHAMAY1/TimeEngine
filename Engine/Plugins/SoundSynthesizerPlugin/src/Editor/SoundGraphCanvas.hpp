#pragma once

#include "../Graph/SoundGraph.hpp"
#include "Utils/TimeGUI.hpp"
#include <memory>

namespace SoundStudio
{

class SoundGraphCanvas
{
public:
    SoundGraphCanvas();

    void Draw(TERef<SoundGraph> graph, float width, float height);

    uint64_t GetSelectedNodeId() const { return m_SelectedNodeId; }

private:
    void DrawNodes(TERef<SoundGraph> graph);
    void DrawConnections(TERef<SoundGraph> graph);
    void DrawContextMenu(TERef<SoundGraph> graph);

private:
    TEVector2 m_Scrolling = {0.0f, 0.0f};
    uint64_t m_SelectedNodeId = 0;
    uint64_t m_ActiveDraggingPinId = 0;
    TEVector2 m_DragStartPos = {0.0f, 0.0f};
};

} // namespace SoundStudio

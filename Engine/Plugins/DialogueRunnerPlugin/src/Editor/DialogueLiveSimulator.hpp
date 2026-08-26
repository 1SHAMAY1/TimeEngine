#pragma once

#include "Graph/DialogueGraph.hpp"
#include "Runtime/StoryInstance.hpp"
#include <cstdint>

class DialogueLiveSimulator
{
public:
    DialogueLiveSimulator() = default;
    ~DialogueLiveSimulator() = default;

    void StartSimulation(const DialogueGraph &graph);
    void StopSimulation();
    bool IsSimulating() const { return m_bIsSimulating; }

    uint64_t GetActiveNodeID() const { return m_bIsSimulating ? m_Story.GetCurrentGraphNodeID() : 0; }

    void DrawSimulatorPanel(const DialogueGraph &graph);

private:
    bool m_bIsSimulating = false;
    StoryInstance m_Story;
    TEString m_LastText;
    TEString m_LastSpeaker;
};
